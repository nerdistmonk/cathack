#ifndef CATHACK_H
#define CATHACK_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 20
#define MAX_ROOMS 8
#define MAX_ENEMIES 12
#define INVENTORY_CAP 8
#define FOV_RADIUS 7
#define SAVE_FILE "cathack_meta.dat"

typedef enum {
    ITEM_FOOD,
    ITEM_DRINK,
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_GOLD,
    ITEM_YARN_BALL,   // Ranged single-target damage
    ITEM_CATNIP_BOMB  // 3x3 AoE sleep bomb
} ItemType;

typedef struct {
    ItemType type;
    char name[32];
    int value;       // Power, Heal, Gold amount, or Damage
    int cost;        // Merchant buy price
    char symbol;
    int color_pair;
} Item;

typedef enum {
    AI_MELEE,
    AI_RANGED,      // Hairball spitter
    AI_COWARD,      // Flees when low HP
    AI_PACK_HUNTER, // +1 ATK per nearby ally
    AI_MERCHANT     // Neutral shopkeeper
} AIType;

typedef struct {
    int x, y;
    int hp, max_hp;
    int atk, def;
    AIType ai;
    char symbol;
    int color_pair;
    char name[32];
    bool alive;
    int sleep_turns;
} Enemy;

typedef struct {
    int persistent_hp;  // Max HP bonus bought with banked gold
    int persistent_atk; // Base ATK bonus bought with banked gold
    int persistent_def; // Base DEF bonus bought with banked gold
    int banked_gold;    // Total accumulated gold across runs
} MetaStats;

typedef struct {
    int x, y;
    int hp, max_hp;
    int atk, def;
    int gold;
    int dungeon_depth;
    Item inventory[INVENTORY_CAP];
    int inv_count;
    Item equipped_weapon;
    Item equipped_armor;
    bool has_weapon;
    bool has_armor;
    int slowed_turns;  // Trapped in litter
} Player;

typedef struct {
    int x, y, w, h;
} Room;

// Global State Exports
extern char map[MAP_HEIGHT][MAP_WIDTH];
extern bool visible[MAP_HEIGHT][MAP_WIDTH];
extern bool explored[MAP_HEIGHT][MAP_WIDTH];
extern Item item_map[MAP_HEIGHT][MAP_WIDTH];
extern bool has_item_map[MAP_HEIGHT][MAP_WIDTH];
extern Enemy enemies[MAX_ENEMIES];
extern int enemy_count;
extern Player player;
extern MetaStats meta_stats;
extern char log_msg[128];
extern int stairs_x, stairs_y;

// Function Prototypes
void set_log(const char *fmt, ...);
void load_meta(void);
void save_meta(void);

void generate_dungeon(void);
void compute_fov(void);

void reset_player_for_run(void);
void bump_attack(Enemy *e);
void move_player(int dx, int dy);
void enemy_turn(void);
void throw_item(void);

void pickup_item(void);
void open_inventory(void);
void open_stat_shop(void);
void open_merchant_shop(void);

void render_game(void);

#endif
