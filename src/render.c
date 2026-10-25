#include "cathack.h"

void render_game(void) {
    clear();

    int wall_color = get_floor_color_pair(player.dungeon_depth);

    // Render Map & Fog of War
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (visible[y][x]) {
                if (has_item_map[y][x]) {
                    attron(COLOR_PAIR(item_map[y][x].color_pair) | A_BOLD);
                    mvaddch(y, x, item_map[y][x].symbol);
                    attroff(COLOR_PAIR(item_map[y][x].color_pair) | A_BOLD);
                } else {
                    char tile = map[y][x];
                    int pair = 0;
                    if (tile == '#') pair = wall_color;
                    else if (tile == '=') pair = 5;
                    else if (tile == '^') pair = 2;

                    if (pair) attron(COLOR_PAIR(pair) | (player.dungeon_depth >= BOSS_START_DEPTH ? A_BOLD : 0));
                    mvaddch(y, x, tile);
                    if (pair) attroff(COLOR_PAIR(pair) | (player.dungeon_depth >= BOSS_START_DEPTH ? A_BOLD : 0));
                }
            } else if (explored[y][x]) {
                attron(A_DIM);
                mvaddch(y, x, map[y][x]);
                attroff(A_DIM);
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }

    // Render Visible Enemies
    for (int i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (e->alive && visible[e->y][e->x]) {
            attron(COLOR_PAIR(e->color_pair) | A_BOLD);
            mvaddch(e->y, e->x, e->symbol);
            attroff(COLOR_PAIR(e->color_pair) | A_BOLD);
        }
    }

    // Dynamic Player Symbol Color based on HP Ratio
    int player_color_pair = 6; // White (> 50% HP)
    float hp_ratio = (float)player.hp / (float)player.max_hp;
    if (hp_ratio <= 0.25f) {
        player_color_pair = 3; // Red (< 25% HP)
    } else if (hp_ratio <= 0.50f) {
        player_color_pair = 1; // Yellow (25% - 50% HP)
    }

    attron(COLOR_PAIR(player_color_pair) | A_BOLD);
    mvaddch(player.y, player.x, '@');
    attroff(COLOR_PAIR(player_color_pair) | A_BOLD);

    // Status Bar & HUD
    int total_atk = player.atk + (player.has_weapon ? player.equipped_weapon.value : 0);
    int total_def = player.def + (player.has_armor ? player.equipped_armor.value : 0);

    const char *latest_log = (log_count > 0) ? log_history[log_count - 1] : "";

    mvprintw(MAP_HEIGHT, 0,
             "HP: %d/%d | ATK: %d (%+d) | DEF: %d (%+d) | Gold: %d | Depth: %d/%d %s",
             player.hp, player.max_hp, total_atk, player.has_weapon ? player.equipped_weapon.value : 0,
             total_def, player.has_armor ? player.equipped_armor.value : 0,
             player.gold, player.dungeon_depth, MAX_DEPTH,
             player.dungeon_depth >= BOSS_START_DEPTH ? "[BOSS RUN]" : "");

    mvprintw(MAP_HEIGHT + 1, 0, "Log: %s", latest_log);
    mvprintw(MAP_HEIGHT + 2, 0,
             "[WASD]: Move | [g]: Get | [i]: Inv | [t]: Throw | [p]: History | [. / >]: Stairs");

    refresh();
}
