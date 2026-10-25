#include "cathack.h"

void render_game(void) {
    clear();

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
                    if (tile == '#') pair = 6;
                    else if (tile == '=') pair = 5;
                    else if (tile == '^') pair = 2;

                    if (pair) attron(COLOR_PAIR(pair));
                    mvaddch(y, x, tile);
                    if (pair) attroff(COLOR_PAIR(pair));
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

    for (int i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (e->alive && visible[e->y][e->x]) {
            attron(COLOR_PAIR(e->color_pair) | A_BOLD);
            mvaddch(e->y, e->x, e->symbol);
            attroff(COLOR_PAIR(e->color_pair) | A_BOLD);
        }
    }

    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddch(player.y, player.x, '@');
    attroff(COLOR_PAIR(1) | A_BOLD);

    int total_atk = player.atk + (player.has_weapon ? player.equipped_weapon.value : 0);
    int total_def = player.def + (player.has_armor ? player.equipped_armor.value : 0);

    mvprintw(MAP_HEIGHT, 0,
             "HP: %d/%d | ATK: %d (%+d) | DEF: %d (%+d) | Gold: %d | Depth: Level %d",
             player.hp, player.max_hp, total_atk, player.has_weapon ? player.equipped_weapon.value : 0,
             total_def, player.has_armor ? player.equipped_armor.value : 0,
             player.gold, player.dungeon_depth);

    mvprintw(MAP_HEIGHT + 1, 0, "Log: %s", log_msg);
    mvprintw(MAP_HEIGHT + 2, 0,
             "[WASD]: Move | [g]: Pick Up | [i]: Inv | [t]: Throw Ranged | [>]: Stairs");

    refresh();
}
