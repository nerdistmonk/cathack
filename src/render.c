#include "cathack.h"

void render_game(void) {
    clear();

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);

    int hud_height = 3;
    int available_h = term_h - hud_height;
    if (available_h < 5) available_h = 5;

    int view_h = (available_h < MAP_HEIGHT) ? available_h : MAP_HEIGHT;
    int view_w = (term_w < MAP_WIDTH) ? term_w : MAP_WIDTH;

    // Center camera on player
    int cam_x = player.x - view_w / 2;
    int cam_y = player.y - view_h / 2;

    if (cam_x < 0) cam_x = 0;
    if (cam_x + view_w > MAP_WIDTH) cam_x = MAP_WIDTH - view_w;

    if (cam_y < 0) cam_y = 0;
    if (cam_y + view_h > MAP_HEIGHT) cam_y = MAP_HEIGHT - view_h;

    // Screen offsets to center the game view if terminal is larger than the map/view
    int screen_offset_x = (term_w > view_w) ? (term_w - view_w) / 2 : 0;
    int screen_offset_y = (available_h > view_h) ? (available_h - view_h) / 2 : 0;

    int wall_color = get_floor_color_pair(player.dungeon_depth);

    // Render Map & Fog of War within Viewport
    for (int vy = 0; vy < view_h; vy++) {
        for (int vx = 0; vx < view_w; vx++) {
            int y = cam_y + vy;
            int x = cam_x + vx;
            int screen_y = screen_offset_y + vy;
            int screen_x = screen_offset_x + vx;

            if (visible[y][x]) {
                if (has_item_map[y][x]) {
                    attron(COLOR_PAIR(item_map[y][x].color_pair) | A_BOLD);
                    mvaddch(screen_y, screen_x, item_map[y][x].symbol);
                    attroff(COLOR_PAIR(item_map[y][x].color_pair) | A_BOLD);
                } else {
                    char tile = map[y][x];
                    int pair = 0;
                    if (tile == '#') pair = wall_color;
                    else if (tile == '=') pair = 5;
                    else if (tile == '^') pair = 2;

                    if (pair) attron(COLOR_PAIR(pair) | (player.dungeon_depth >= BOSS_START_DEPTH ? A_BOLD : 0));
                    mvaddch(screen_y, screen_x, tile);
                    if (pair) attroff(COLOR_PAIR(pair) | (player.dungeon_depth >= BOSS_START_DEPTH ? A_BOLD : 0));
                }
            } else if (explored[y][x]) {
                attron(A_DIM);
                mvaddch(screen_y, screen_x, map[y][x]);
                attroff(A_DIM);
            } else {
                mvaddch(screen_y, screen_x, ' ');
            }
        }
    }

    // Render Visible Enemies within Viewport
    for (int i = 0; i < enemy_count; i++) {
        Enemy *e = &enemies[i];
        if (e->alive && visible[e->y][e->x]) {
            if (e->x >= cam_x && e->x < cam_x + view_w &&
                e->y >= cam_y && e->y < cam_y + view_h) {
                int screen_y = screen_offset_y + (e->y - cam_y);
                int screen_x = screen_offset_x + (e->x - cam_x);

                attron(COLOR_PAIR(e->color_pair) | A_BOLD);
                mvaddch(screen_y, screen_x, e->symbol);
                attroff(COLOR_PAIR(e->color_pair) | A_BOLD);
            }
        }
    }

    // Render Player within Viewport if visible
    if (player.x >= cam_x && player.x < cam_x + view_w &&
        player.y >= cam_y && player.y < cam_y + view_h) {
        int screen_y = screen_offset_y + (player.y - cam_y);
        int screen_x = screen_offset_x + (player.x - cam_x);

        int player_color_pair = 6; // White (> 50% HP)
        float hp_ratio = (float)player.hp / (float)player.max_hp;
        if (hp_ratio <= 0.25f) {
            player_color_pair = 3; // Red (< 25% HP)
        } else if (hp_ratio <= 0.50f) {
            player_color_pair = 1; // Yellow (25% - 50% HP)
        }

        attron(COLOR_PAIR(player_color_pair) | A_BOLD);
        mvaddch(screen_y, screen_x, '@');
        attroff(COLOR_PAIR(player_color_pair) | A_BOLD);
    }

    // Status Bar & HUD dynamically pinned at bottom
    int total_atk = player.atk + (player.has_weapon ? player.equipped_weapon.value : 0);
    int total_def = player.def + (player.has_armor ? player.equipped_armor.value : 0);
    const char *latest_log = (log_count > 0) ? log_history[log_count - 1] : "";

    mvprintw(term_h - 3, 0,
             "HP: %d/%d | ATK: %d (%+d) | DEF: %d (%+d) | Gold: %d | Depth: %d/%d %s",
             player.hp, player.max_hp, total_atk, player.has_weapon ? player.equipped_weapon.value : 0,
             total_def, player.has_armor ? player.equipped_armor.value : 0,
             player.gold, player.dungeon_depth, MAX_DEPTH,
             player.dungeon_depth >= BOSS_START_DEPTH ? "[BOSS RUN]" : "");

    mvprintw(term_h - 2, 0, "Log: %s", latest_log);
    mvprintw(term_h - 1, 0,
             "[WASD]: Move | [g]: Get | [i]: Inv | [t]: Throw | [p]: History | [. / >]: Stairs");

    refresh();
}