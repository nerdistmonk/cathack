#include "cathack.h"

char map[MAP_HEIGHT][MAP_WIDTH];
bool visible[MAP_HEIGHT][MAP_WIDTH];
bool explored[MAP_HEIGHT][MAP_WIDTH];
Item item_map[MAP_HEIGHT][MAP_WIDTH];
bool has_item_map[MAP_HEIGHT][MAP_WIDTH];
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
int stairs_x, stairs_y;

static void cast_ray(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int cx = x0, cy = y0;
    while (1) {
        int dist_sq = (cx - x0) * (cx - x0) + (cy - y0) * (cy - y0);
        if (dist_sq > FOV_RADIUS * FOV_RADIUS) break;

        if (cx >= 0 && cx < MAP_WIDTH && cy >= 0 && cy < MAP_HEIGHT) {
            visible[cy][cx] = true;
            explored[cy][cx] = true;
            if (map[cy][cx] == '#') break;
        } else {
            break;
        }

        if (cx == x1 && cy == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; cx += sx; }
        if (e2 < dx)  { err += dx; cy += sy; }
    }
}

void compute_fov(void) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            visible[y][x] = false;
        }
    }

    int px = player.x, py = player.y;
    for (int x = px - FOV_RADIUS; x <= px + FOV_RADIUS; x++) {
        cast_ray(px, py, x, py - FOV_RADIUS);
        cast_ray(px, py, x, py + FOV_RADIUS);
    }
    for (int y = py - FOV_RADIUS; y <= py + FOV_RADIUS; y++) {
        cast_ray(px, py, px - FOV_RADIUS, y);
        cast_ray(px, py, px + FOV_RADIUS, y);
    }
}

void generate_dungeon(void) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = '#';
            visible[y][x] = false;
            explored[y][x] = false;
            has_item_map[y][x] = false;
        }
    }

    Room rooms[MAX_ROOMS];
    int room_count = 0;

    for (int i = 0; i < MAX_ROOMS; i++) {
        int w = 6 + rand() % 8;
        int h = 4 + rand() % 5;
        int x = 1 + rand() % (MAP_WIDTH - w - 2);
        int y = 1 + rand() % (MAP_HEIGHT - h - 2);

        bool overlap = false;
        for (int r = 0; r < room_count; r++) {
            if (x < rooms[r].x + rooms[r].w && x + w > rooms[r].x &&
                y < rooms[r].y + rooms[r].h && y + h > rooms[r].y) {
                overlap = true;
                break;
            }
        }
        if (overlap) continue;

        for (int ry = y; ry < y + h; ry++) {
            for (int rx = x; rx < x + w; rx++) {
                map[ry][rx] = '.';
            }
        }

        if (room_count > 0) {
            int prev_cx = rooms[room_count - 1].x + rooms[room_count - 1].w / 2;
            int prev_cy = rooms[room_count - 1].y + rooms[room_count - 1].h / 2;
            int cx = x + w / 2;
            int cy = y + h / 2;

            int curr_x = prev_cx;
            while (curr_x != cx) {
                map[prev_cy][curr_x] = '.';
                curr_x += (cx > prev_cx) ? 1 : -1;
            }
            int curr_y = prev_cy;
            while (curr_y != cy) {
                map[curr_y][cx] = '.';
                curr_y += (cy > prev_cy) ? 1 : -1;
            }
        }

        rooms[room_count++] = (Room){x, y, w, h};
    }

    player.x = rooms[0].x + 1;
    player.y = rooms[0].y + 1;

    stairs_x = rooms[room_count - 1].x + rooms[room_count - 1].w / 2;
    stairs_y = rooms[room_count - 1].y + rooms[room_count - 1].h / 2;
    map[stairs_y][stairs_x] = '>';

    for (int r = 1; r < room_count; r++) {
        if (rand() % 2 == 0) {
            int hx = rooms[r].x + 2 + rand() % (rooms[r].w - 3);
            int hy = rooms[r].y + 2 + rand() % (rooms[r].h - 3);
            map[hy][hx] = (rand() % 2 == 0) ? '=' : '^';
        }
    }

    enemy_count = 0;
    int depth = player.dungeon_depth;

    if (depth % 3 == 0 && room_count > 2) {
        Enemy *m = &enemies[enemy_count++];
        m->x = rooms[1].x + rooms[1].w / 2;
        m->y = rooms[1].y + rooms[1].h / 2;
        m->alive = true;
        m->ai = AI_MERCHANT;
        m->symbol = 'M';
        m->color_pair = 1;
        snprintf(m->name, sizeof(m->name), "Alley Merchant");
        m->hp = m->max_hp = 100;
        m->atk = 20;
        m->def = 10;
        m->sleep_turns = 0;
    }

    for (int r = 1; r < room_count; r++) {
        int rx = rooms[r].x + 1 + rand() % (rooms[r].w - 2);
        int ry = rooms[r].y + 1 + rand() % (rooms[r].h - 2);

        if (enemy_count < MAX_ENEMIES && map[ry][rx] == '.') {
            Enemy *e = &enemies[enemy_count++];
            e->x = rx;
            e->y = ry;
            e->alive = true;
            e->sleep_turns = 0;

            int type = rand() % 4;
            if (type == 0) {
                snprintf(e->name, sizeof(e->name), "Stray Cat");
                e->symbol = 'c';
                e->color_pair = 3;
                e->ai = AI_COWARD;
                e->hp = e->max_hp = 4 + depth * 2;
                e->atk = 2 + depth * 1;
                e->def = 0 + depth;
            } else if (type == 1) {
                snprintf(e->name, sizeof(e->name), "Spitting Tomcat");
                e->symbol = 's';
                e->color_pair = 5;
                e->ai = AI_RANGED;
                e->hp = e->max_hp = 6 + depth * 2;
                e->atk = 3 + depth * 1;
                e->def = 0 + depth;
            } else if (type == 2) {
                snprintf(e->name, sizeof(e->name), "Feral Alley-Cat");
                e->symbol = 'C';
                e->color_pair = 1;
                e->ai = AI_PACK_HUNTER;
                e->hp = e->max_hp = 8 + depth * 3;
                e->atk = 3 + depth * 2;
                e->def = 1 + depth;
            } else {
                snprintf(e->name, sizeof(e->name), "Dungeon Panther");
                e->symbol = 'P';
                e->color_pair = 4;
                e->ai = AI_MELEE;
                e->hp = e->max_hp = 12 + depth * 4;
                e->atk = 5 + depth * 2;
                e->def = 2 + depth;
            }
        }

        if (rand() % 2 == 0) {
            int ix = rooms[r].x + 1 + rand() % (rooms[r].w - 2);
            int iy = rooms[r].y + 1 + rand() % (rooms[r].h - 2);
            if (map[iy][ix] == '.') {
                has_item_map[iy][ix] = true;
                int itype = rand() % 5;
                if (itype == 0) {
                    item_map[iy][ix] = (Item){ITEM_FOOD, "Can of Tuna", 10 + depth * 2, 0, '%', 2};
                } else if (itype == 1) {
                    item_map[iy][ix] = (Item){ITEM_DRINK, "Bowl of Milk", 15 + depth * 3, 0, '!', 5};
                } else if (itype == 2) {
                    item_map[iy][ix] = (Item){ITEM_YARN_BALL, "Yarn Ball", 8 + depth * 3, 0, '*', 1};
                } else if (itype == 3) {
                    item_map[iy][ix] = (Item){ITEM_CATNIP_BOMB, "Catnip Bomb", 5, 0, '&', 2};
                } else {
                    item_map[iy][ix] = (Item){ITEM_GOLD, "Gold Coins", 15 + depth * 15, 0, '$', 1};
                }
            }
        }
    }

    compute_fov();
}
