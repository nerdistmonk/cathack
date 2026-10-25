#include "cathack.h"

Player player;

void reset_player_for_run(void) {
    player.max_hp = 30 + meta_stats.persistent_hp;
    player.hp = player.max_hp;
    player.atk = 5 + meta_stats.persistent_atk;
    player.def = 1 + meta_stats.persistent_def;
    player.gold = 0;
    player.dungeon_depth = 1;
    player.inv_count = 0;
    player.has_weapon = false;
    player.has_armor = false;
    player.slowed_turns = 0;

    log_count = 0;
    add_log("Welcome to CatHack! Press WASD to move/attack, [p] for log.");

    init_mystery_items();
    generate_dungeon();
}

static void spawn_dropped_loot(int x, int y, int depth) {
    if (has_item_map[y][x]) return;

    has_item_map[y][x] = true;
    int roll = rand() % 100;

    if (roll < 45) {
        int amount = 15 + depth * 20 + (rand() % 15);
        item_map[y][x] = (Item){ITEM_GOLD, "Shiny Gold", amount, 0, '$', 1, -1};
    } else if (roll < 70) {
        char wpn_name[32];
        int bonus = 2 + depth * 2;
        snprintf(wpn_name, sizeof(wpn_name), "Claw Ring +%d", bonus);
        item_map[y][x] = (Item){ITEM_WEAPON, "", bonus, 0, '(', 3, -1};
        snprintf(item_map[y][x].name, sizeof(item_map[y][x].name), "%s", wpn_name);
    } else if (roll < 90) {
        char arm_name[32];
        int bonus = 1 + depth * 2;
        snprintf(arm_name, sizeof(arm_name), "Leather Collar +%d", bonus);
        item_map[y][x] = (Item){ITEM_ARMOR, "", bonus, 0, '[', 5, -1};
        snprintf(item_map[y][x].name, sizeof(item_map[y][x].name), "%s", arm_name);
    } else {
        item_map[y][x] = (Item){ITEM_YARN_BALL, "Yarn Ball", 10 + depth * 2, 0, '*', 1, -1};
    }
}

void bump_attack(Enemy *e) {
    if (e->ai == AI_MERCHANT) {
        add_log("You hit the Alley Merchant! He turns hostile!");
    }

    int total_atk = player.atk + (player.has_weapon ? player.equipped_weapon.value : 0);
    int damage = total_atk - e->def;
    if (damage < 1) damage = 1;

    e->hp -= damage;
    e->sleep_turns = 0;

    if (e->hp <= 0) {
        e->alive = false;
        add_log("You hit %s for %d dmg and defeated it!", e->name, damage);
        spawn_dropped_loot(e->x, e->y, player.dungeon_depth);
    } else {
        add_log("You hit %s for %d dmg.", e->name, damage);
    }
}

void enemy_turn(void) {
    for (int i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (!e->alive || e->ai == AI_MERCHANT) continue;

        if (e->sleep_turns > 0) {
            e->sleep_turns--;
            continue;
        }

        int dx = abs(e->x - player.x);
        int dy = abs(e->y - player.y);

        if (dx <= FOV_RADIUS && dy <= FOV_RADIUS && visible[e->y][e->x]) {
            if (dx <= 1 && dy <= 1) {
                int bonus_atk = 0;
                if (e->ai == AI_PACK_HUNTER) {
                    for (int j = 0; j < enemy_count; j++) {
                        if (i != j && enemies[j].alive &&
                            abs(enemies[j].x - e->x) <= 2 && abs(enemies[j].y - e->y) <= 2) {
                            bonus_atk++;
                        }
                    }
                }

                int total_def = player.def + (player.has_armor ? player.equipped_armor.value : 0);
                int damage = (e->atk + bonus_atk) - total_def;
                if (damage < 1) damage = 1;
                player.hp -= damage;
                add_log("%s attacks you for %d damage!", e->name, damage);

            } else if (e->ai == AI_RANGED && dx <= 4 && dy <= 4) {
                int damage = e->atk - (player.has_armor ? player.equipped_armor.value : 0);
                if (damage < 1) damage = 1;
                player.hp -= damage;
                add_log("%s spits a hairball at you for %d dmg!", e->name, damage);

            } else {
                int mx = 0, my = 0;
                if (e->ai == AI_COWARD && e->hp < e->max_hp / 3) {
                    if (player.x > e->x) mx = -1; else if (player.x < e->x) mx = 1;
                    if (player.y > e->y) my = -1; else if (player.y < e->y) my = 1;
                } else {
                    if (player.x > e->x) mx = 1; else if (player.x < e->x) mx = -1;
                    if (player.y > e->y) my = 1; else if (player.y < e->y) my = -1;
                }

                int nx = e->x + mx;
                int ny = e->y + my;
                if ((map[ny][nx] == '.' || map[ny][nx] == '=' || map[ny][nx] == '^') &&
                    (nx != player.x || ny != player.y)) {
                    e->x = nx;
                    e->y = ny;
                }
            }
        }
    }
}

void move_player(int dx, int dy) {
    if (player.slowed_turns > 0) {
        player.slowed_turns--;
        add_log("Stuck in cat litter! (1 turn penalty remaining)");
        enemy_turn();
        return;
    }

    int nx = player.x + dx;
    int ny = player.y + dy;

    for (int i = 0; i < enemy_count; i++) {
        if (enemies[i].alive && enemies[i].x == nx && enemies[i].y == ny) {
            if (enemies[i].ai == AI_MERCHANT) {
                open_merchant_shop();
                return;
            }
            bump_attack(&enemies[i]);
            enemy_turn();
            return;
        }
    }

    char tile = map[ny][nx];
    if (tile == '.' || tile == '>' || tile == '=' || tile == '^') {
        player.x = nx;
        player.y = ny;

        if (tile == '=') {
            add_log("Slipped on a water puddle!");
            int sx = nx + dx, sy = ny + dy;
            if (map[sy][sx] == '.' || map[sy][sx] == '>') {
                player.x = sx;
                player.y = sy;
            }
        } else if (tile == '^') {
            add_log("Stepped into sticky cat litter! Slowed down.");
            player.slowed_turns = 1;
        }

        if (has_item_map[player.y][player.x]) {
            Item *item = &item_map[player.y][player.x];
            if (item->type == ITEM_GOLD) {
                player.gold += item->value;
                add_log("Picked up %d Gold Coins!", item->value);
                has_item_map[player.y][player.x] = false;
            } else {
                add_log("Standing on %s. Press 'g' to pick up.", item->name);
            }
        }

        compute_fov();
        enemy_turn();
    }
}

void throw_item(void) {
    int throw_idx = -1;
    for (int i = 0; i < player.inv_count; i++) {
        if (player.inventory[i].type == ITEM_YARN_BALL ||
            player.inventory[i].type == ITEM_CATNIP_BOMB) {
            throw_idx = i;
            break;
        }
    }

    if (throw_idx == -1) {
        add_log("No throwables (Yarn Ball / Catnip Bomb) in inventory!");
        return;
    }

    Item item = player.inventory[throw_idx];

    for (int i = throw_idx; i < player.inv_count - 1; i++) {
        player.inventory[i] = player.inventory[i + 1];
    }
    player.inv_count--;

    int target_idx = -1;
    int min_dist = 999;
    for (int i = 0; i < enemy_count; i++) {
        if (enemies[i].alive && visible[enemies[i].y][enemies[i].x]) {
            int d = abs(enemies[i].x - player.x) + abs(enemies[i].y - player.y);
            if (d < min_dist) {
                min_dist = d;
                target_idx = i;
            }
        }
    }

    if (target_idx == -1) {
        add_log("No visible targets to throw at!");
        return;
    }

    Enemy *target = &enemies[target_idx];

    if (item.type == ITEM_YARN_BALL) {
        target->hp -= item.value;
        target->sleep_turns = 0;
        add_log("Threw Yarn Ball at %s for %d damage!", target->name, item.value);
        if (target->hp <= 0) {
            target->alive = false;
            spawn_dropped_loot(target->x, target->y, player.dungeon_depth);
        }
    } else if (item.type == ITEM_CATNIP_BOMB) {
        for (int i = 0; i < enemy_count; i++) {
            if (enemies[i].alive &&
                abs(enemies[i].x - target->x) <= 1 &&
                abs(enemies[i].y - target->y) <= 1) {
                enemies[i].sleep_turns = item.value;
            }
        }
        add_log("Catnip Bomb exploded! Nearby enemies fell asleep!");
    }

    enemy_turn();
}
