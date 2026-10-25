#include "cathack.h"

MetaStats meta_stats = {0, 0, 0, 0};
char log_msg[128] = "Welcome to CatHack! Press WASD to move/attack.";

void set_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(log_msg, sizeof(log_msg), fmt, args);
    va_end(args);
}

static void get_save_path(char *out_path, size_t size) {
    const char *home = getenv("HOME");
    if (home && home[0] != '\0') {
        snprintf(out_path, size, "%s/.cathack_meta.dat", home);
    } else {
        snprintf(out_path, size, "%s", SAVE_FILE);
    }
}

void load_meta(void) {
    char path[256];
    get_save_path(path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (f) {
        fread(&meta_stats, sizeof(MetaStats), 1, f);
        fclose(f);
    }
}

void save_meta(void) {
    char path[256];
    get_save_path(path, sizeof(path));
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(&meta_stats, sizeof(MetaStats), 1, f);
        fclose(f);
    }
}

void pickup_item(void) {
    if (!has_item_map[player.y][player.x]) {
        set_log("Nothing here to pick up.");
        return;
    }

    if (player.inv_count >= INVENTORY_CAP) {
        set_log("Inventory full!");
        return;
    }

    Item item = item_map[player.y][player.x];
    player.inventory[player.inv_count++] = item;
    has_item_map[player.y][player.x] = false;
    set_log("Picked up %s.", item.name);
}

void open_inventory(void) {
    clear();
    mvprintw(1, 2, "=== INVENTORY (Press 1-8 to use/equip, or ESC/q to exit) ===");
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
    if (ch >= '1' && ch < '1' + player.inv_count) {
        int idx = ch - '1';
        Item item = player.inventory[idx];

        for (int i = idx; i < player.inv_count - 1; i++) {
            player.inventory[i] = player.inventory[i + 1];
        }
        player.inv_count--;

        if (item.type == ITEM_FOOD || item.type == ITEM_DRINK) {
            player.hp += item.value;
            if (player.hp > player.max_hp) player.hp = player.max_hp;
            set_log("Consumed %s and restored health!", item.name);
        } else if (item.type == ITEM_WEAPON) {
            if (player.has_weapon && player.inv_count < INVENTORY_CAP) {
                player.inventory[player.inv_count++] = player.equipped_weapon;
            }
            player.equipped_weapon = item;
            player.has_weapon = true;
            set_log("Equipped %s.", item.name);
        } else if (item.type == ITEM_ARMOR) {
            if (player.has_armor && player.inv_count < INVENTORY_CAP) {
                player.inventory[player.inv_count++] = player.equipped_armor;
            }
            player.equipped_armor = item;
            player.has_armor = true;
            set_log("Equipped %s.", item.name);
        }
    }
}

void open_merchant_shop(void) {
    bool shopping = true;
    while (shopping) {
        clear();
        mvprintw(2, 2, "=== ALLEY MERCHANT SHOP ===");
        mvprintw(4, 4, "Current Run Gold: %d", player.gold);

        mvprintw(6, 4, "[1] Salmon Supreme (+25 HP)        - Cost: 40 Gold");
        mvprintw(8, 4, "[2] Sharp Claw Ring (+4 ATK)       - Cost: 80 Gold");
        mvprintw(10, 4, "[3] Spiked Collar (+3 DEF)         - Cost: 75 Gold");
        mvprintw(12, 4, "[4] Catnip Sleep Bomb (&)           - Cost: 30 Gold");
        mvprintw(15, 4, "[q/ESC] Leave Shop");
        refresh();

        int ch = getch();
        if (ch == '1' && player.gold >= 40) {
            player.gold -= 40;
            player.hp += 25;
            if (player.hp > player.max_hp) player.hp = player.max_hp;
            set_log("Bought Salmon Supreme and fully healed!");
        } else if (ch == '2' && player.gold >= 80) {
            player.gold -= 80;
            player.equipped_weapon = (Item){ITEM_WEAPON, "Sharp Claw Ring +4", 4, 0, '(', 3};
            player.has_weapon = true;
            set_log("Bought and equipped Sharp Claw Ring!");
        } else if (ch == '3' && player.gold >= 75) {
            player.gold -= 75;
            player.equipped_armor = (Item){ITEM_ARMOR, "Spiked Collar +3", 3, 0, '[', 5};
            player.has_armor = true;
            set_log("Bought and equipped Spiked Collar!");
        } else if (ch == '4' && player.gold >= 30 && player.inv_count < INVENTORY_CAP) {
            player.gold -= 30;
            player.inventory[player.inv_count++] = (Item){ITEM_CATNIP_BOMB, "Catnip Bomb", 5, 0, '&', 2};
            set_log("Bought Catnip Bomb!");
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

        mvprintw(14, 4, "[SPACE/ENTER] Start New Run");
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
        } else if (ch == ' ' || ch == '\n' || ch == '\r') {
            in_shop = false;
        }
    }
}
