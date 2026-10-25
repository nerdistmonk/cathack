#include "cathack.h"

MetaStats meta_stats = {0, 0, 0, 0};
char log_history[LOG_HISTORY_CAP][128];
int log_count = 0;

MysteryEffect mystery_effects[MYSTERY_COUNT];
const char *MYSTERY_NAMES[MYSTERY_COUNT] = {
    "Red Tin",
    "Blue Saucer",
    "Murky Bottle",
    "Strange Kibble",
    "Fizzy Milk"
};

void add_log(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (log_count < LOG_HISTORY_CAP) {
        strncpy(log_history[log_count++], buf, 128);
    } else {
        for (int i = 0; i < LOG_HISTORY_CAP - 1; i++) {
            strncpy(log_history[i], log_history[i + 1], 128);
        }
        strncpy(log_history[LOG_HISTORY_CAP - 1], buf, 128);
    }
}

void open_log_history(void) {
    clear();
    mvprintw(1, 2, "=== COMBAT & MESSAGE HISTORY (Press ESC/q to close) ===");
    int start = (log_count > 18) ? log_count - 18 : 0;
    for (int i = start, line = 3; i < log_count; i++, line++) {
        mvprintw(line, 4, "- %s", log_history[i]);
    }
    refresh();
    while (1) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q' || ch == 27 || ch == 'p' || ch == 'P') break;
    }
}

static void get_file_path(char *out_path, size_t size, const char *filename) {
    const char *home = getenv("HOME");
    if (home && home[0] != '\0') {
        snprintf(out_path, size, "%s/.%s", home, filename);
    } else {
        snprintf(out_path, size, "%s", filename);
    }
}

void load_meta(void) {
    char path[256];
    get_file_path(path, sizeof(path), SAVE_FILE);
    FILE *f = fopen(path, "rb");
    if (f) {
        fread(&meta_stats, sizeof(MetaStats), 1, f);
        fclose(f);
    }
}

void save_meta(void) {
    char path[256];
    get_file_path(path, sizeof(path), SAVE_FILE);
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(&meta_stats, sizeof(MetaStats), 1, f);
        fclose(f);
    }
}

void record_high_score(bool won) {
    char path[256];
    get_file_path(path, sizeof(path), SCORE_FILE);

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char date_buf[32];
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M", tm_info);

    FILE *f = fopen(path, "a");
    if (f) {
        fprintf(f, "%s|%d|%d|%d\n", date_buf, player.dungeon_depth, player.gold, won ? 1 : 0);
        fclose(f);
    }
}

static int compare_scores(const void *a, const void *b) {
    const HighScore *sa = (const HighScore *)a;
    const HighScore *sb = (const HighScore *)b;

    if (sb->depth != sa->depth) {
        return sb->depth - sa->depth; // Highest level first
    }
    if (sb->won != sa->won) {
        return sb->won - sa->won;     // Victory wins tie-breaker
    }
    return sb->gold - sa->gold;       // Highest gold first
}

void display_high_scores(void) {
    clear();
    mvprintw(2, 2, "=== CATHACK HALL OF FAME (HIGH SCORES) ===");
    mvprintw(4, 4, "%-18s %-12s %-12s %-10s", "DATE", "MAX DEPTH", "GOLD", "RESULT");
    mvprintw(5, 4, "----------------------------------------------------");

    char path[256];
    get_file_path(path, sizeof(path), SCORE_FILE);
    FILE *f = fopen(path, "r");

    if (f) {
        HighScore scores[100];
        int count = 0;
        char line[128];

        while (fgets(line, sizeof(line), f) && count < 100) {
            int won_int = 0;
            if (sscanf(line, "%31[^|]|%d|%d|%d",
                       scores[count].date,
                       &scores[count].depth,
                       &scores[count].gold,
                       &won_int) == 4) {
                scores[count].won = (won_int != 0);
                count++;
            }
        }
        fclose(f);

        // Sort high scores descending by depth and gold
        qsort(scores, count, sizeof(HighScore), compare_scores);

        int max_show = (count < 12) ? count : 12;
        for (int i = 0; i < max_show; i++) {
            mvprintw(6 + i, 4, "%-18s Level %-6d %-12d %-10s",
                     scores[i].date, scores[i].depth, scores[i].gold,
                     scores[i].won ? "VICTORY" : "DIED");
        }
    } else {
        mvprintw(7, 4, "(No high score records found yet)");
    }

    mvprintw(20, 4, "Press any key to return...");
    refresh();
    getch();
}

void init_mystery_items(void) {
    MysteryEffect pool[MYSTERY_COUNT] = {
        MYSTERY_HEAL_LARGE,
        MYSTERY_HEAL_SMALL,
        MYSTERY_POISON,
        MYSTERY_BUFF_ATK,
        MYSTERY_BUFF_DEF
    };
    for (int i = MYSTERY_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        MysteryEffect tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }
    for (int i = 0; i < MYSTERY_COUNT; i++) {
        mystery_effects[i] = pool[i];
    }
}

void pickup_item(void) {
    if (!has_item_map[player.y][player.x]) {
        add_log("Nothing here to pick up.");
        return;
    }

    if (player.inv_count >= INVENTORY_CAP) {
        add_log("Inventory full!");
        return;
    }

    Item item = item_map[player.y][player.x];
    player.inventory[player.inv_count++] = item;
    has_item_map[player.y][player.x] = false;
    add_log("Picked up %s.", item.name);
}

void open_inventory(void) {
    bool in_inv = true;
    while (in_inv) {
        clear();
        mvprintw(1, 2, "=== INVENTORY (Press 1-8 to use/equip, or ESC/q/i to exit) ===");
        for (int i = 0; i < player.inv_count; i++) {
            Item *item = &player.inventory[i];
            mvprintw(3 + i, 4, "[%c] %s (Power/Val: %d)", '1' + i, item->name, item->value);
        }
        if (player.inv_count == 0) {
            mvprintw(4, 4, "(Inventory is empty)");
        }

        mvprintw(14, 2, "Equipped Weapon: %s", player.has_weapon ? player.equipped_weapon.name : "None");
        mvprintw(15, 2, "Equipped Armor:  %s", player.has_armor ? player.equipped_armor.name : "None");
        refresh();

        int ch = getch();
        if (ch == 'q' || ch == 'Q' || ch == 27 || ch == 'i' || ch == 'I') {
            in_inv = false;
        } else if (ch >= '1' && ch < '1' + player.inv_count) {
            int idx = ch - '1';
            Item item = player.inventory[idx];

            for (int i = idx; i < player.inv_count - 1; i++) {
                player.inventory[i] = player.inventory[i + 1];
            }
            player.inv_count--;

            if (item.type == ITEM_FOOD || item.type == ITEM_DRINK) {
                player.hp += item.value;
                if (player.hp > player.max_hp) player.hp = player.max_hp;
                add_log("Consumed %s and restored health!", item.name);
            } else if (item.type == ITEM_MYSTERY) {
                MysteryEffect effect = mystery_effects[item.mystery_id];
                if (effect == MYSTERY_HEAL_LARGE) {
                    player.hp += 30;
                    if (player.hp > player.max_hp) player.hp = player.max_hp;
                    add_log("Ate %s: Tasted delicious! (+30 HP)", item.name);
                } else if (effect == MYSTERY_HEAL_SMALL) {
                    player.hp += 12;
                    if (player.hp > player.max_hp) player.hp = player.max_hp;
                    add_log("Ate %s: Tasted okay. (+12 HP)", item.name);
                } else if (effect == MYSTERY_POISON) {
                    player.hp -= 8;
                    add_log("Ate %s: Ugh, poisonous! (-8 HP)", item.name);
                } else if (effect == MYSTERY_BUFF_ATK) {
                    player.atk += 2;
                    add_log("Ate %s: Felt a surge of feral power! (+2 ATK)", item.name);
                } else if (effect == MYSTERY_BUFF_DEF) {
                    player.def += 2;
                    add_log("Ate %s: Skin hardened like iron! (+2 DEF)", item.name);
                }
            } else if (item.type == ITEM_WEAPON) {
                if (player.has_weapon && player.inv_count < INVENTORY_CAP) {
                    player.inventory[player.inv_count++] = player.equipped_weapon;
                }
                player.equipped_weapon = item;
                player.has_weapon = true;
                add_log("Equipped %s.", item.name);
            } else if (item.type == ITEM_ARMOR) {
                if (player.has_armor && player.inv_count < INVENTORY_CAP) {
                    player.inventory[player.inv_count++] = player.equipped_armor;
                }
                player.equipped_armor = item;
                player.has_armor = true;
                add_log("Equipped %s.", item.name);
            }
        }
    }
}

void open_merchant_shop(void) {
    bool shopping = true;
    while (shopping) {
        clear();
        int heal_amount = 40 + player.dungeon_depth * 5;

        mvprintw(2, 2, "=== ALLEY MERCHANT SHOP ===");
        mvprintw(4, 4, "Current Run Gold: %d", player.gold);

        mvprintw(6, 4, "[1] Salmon Supreme (+%d HP)       - Cost: 40 Gold", heal_amount);
        mvprintw(8, 4, "[2] Sharp Claw Ring (+4 ATK)       - Cost: 80 Gold");
        mvprintw(10, 4, "[3] Spiked Collar (+3 DEF)         - Cost: 75 Gold");
        mvprintw(12, 4, "[4] Catnip Sleep Bomb (&)           - Cost: 30 Gold");
        mvprintw(15, 4, "[q/ESC] Leave Shop");
        refresh();

        int ch = getch();
        if (ch == '1' && player.gold >= 40) {
            player.gold -= 40;
            player.hp += heal_amount;
            if (player.hp > player.max_hp) player.hp = player.max_hp;
            add_log("Bought Salmon Supreme and restored health!");
        } else if (ch == '2' && player.gold >= 80) {
            player.gold -= 80;
            player.equipped_weapon = (Item){ITEM_WEAPON, "Sharp Claw Ring +4", 4, 0, '(', 3, -1};
            player.has_weapon = true;
            add_log("Bought and equipped Sharp Claw Ring!");
        } else if (ch == '3' && player.gold >= 75) {
            player.gold -= 75;
            player.equipped_armor = (Item){ITEM_ARMOR, "Spiked Collar +3", 3, 0, '[', 5, -1};
            player.has_armor = true;
            add_log("Bought and equipped Spiked Collar!");
        } else if (ch == '4' && player.gold >= 30 && player.inv_count < INVENTORY_CAP) {
            player.gold -= 30;
            player.inventory[player.inv_count++] = (Item){ITEM_CATNIP_BOMB, "Catnip Bomb", 5, 0, '&', 2, -1};
            add_log("Bought Catnip Bomb!");
        } else if (ch == 'q' || ch == 27) {
            shopping = false;
        }
    }
}

void open_stat_shop(void) {
    bool in_shop = true;
    while (in_shop) {
        clear();
        mvprintw(2, 2, "=== CATHACK STAT SHOP (Persistent Meta Upgrades) ===");
        mvprintw(4, 4, "Banked Gold: %d", meta_stats.banked_gold);

        int hp_cost = 50 + meta_stats.persistent_hp * 5;
        int atk_cost = 100 + meta_stats.persistent_atk * 20;
        int def_cost = 100 + meta_stats.persistent_def * 20;

        mvprintw(6, 4, "[1] Upgrade Max HP  (+5 Max HP)  - Cost: %d Gold  (Current Bonus: +%d)",
                 hp_cost, meta_stats.persistent_hp);
        mvprintw(8, 4, "[2] Upgrade Base ATK (+1 ATK)     - Cost: %d Gold  (Current Bonus: +%d)",
                 atk_cost, meta_stats.persistent_atk);
        mvprintw(10, 4, "[3] Upgrade Base DEF (+1 DEF)     - Cost: %d Gold  (Current Bonus: +%d)",
                 def_cost, meta_stats.persistent_def);

        mvprintw(13, 4, "[H] View High Scores / Hall of Fame");
        mvprintw(15, 4, "[SPACE/ENTER] Start New Run");
        refresh();

        int ch = getch();
        if (ch == '1' && meta_stats.banked_gold >= hp_cost) {
            meta_stats.banked_gold -= hp_cost;
            meta_stats.persistent_hp += 5;
            save_meta();
        } else if (ch == '2' && meta_stats.banked_gold >= atk_cost) {
            meta_stats.banked_gold -= atk_cost;
            meta_stats.persistent_atk += 1;
            save_meta();
        } else if (ch == '3' && meta_stats.banked_gold >= def_cost) {
            meta_stats.banked_gold -= def_cost;
            meta_stats.persistent_def += 1;
            save_meta();
        } else if (ch == 'h' || ch == 'H') {
            display_high_scores();
        } else if (ch == ' ' || ch == '\n' || ch == '\r') {
            in_shop = false;
        }
    }
}
