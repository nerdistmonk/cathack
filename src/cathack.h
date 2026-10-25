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
#define MAX_ENEMIES 14
#define INVENTORY_CAP 8
#define FOV_RADIUS 7
#define MAX_DEPTH 30
#define BOSS_START_DEPTH 26
#define LOG_HISTORY_CAP 50
#define MYSTERY_COUNT 5
#define SAVE_FILE "cathack_meta.dat"
#define SCORE_FILE "cathack_scores.txt"

typedef enum {
    ITEM_FOOD,
    ITEM_DRINK,
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_GOLD,
    ITEM_YARN_BALL,   // Ranged single-target damage
    ITEM_CATNIP_BOMB, // 3x3 AoE sleep bomb
    ITEM_MYSTERY      // Unidentified consumable
} ItemType;

typedef enum {
    MYSTERY_HEAL_LARGE,
    MYSTERY_HEAL_SMALL,
    MYSTERY_POISON,
    MYSTERY_BUFF_ATK,
    MYSTERY_BUFF_DEF
} MysteryEffect;

typedef struct {
    ItemType type;
    char name[32];
    int value;       // Power, Heal, Gold amount, or Damage
    int cost;        // Merchant buy price
    char symbol;
    int color_pair;
    int mystery_id;  // 0..4 for unidentified items
} Item;

typedef enum {
    AI_MELEE,
    AI_RANGED,      // Hairball spitter
    AI_COWARD,      // Flees when low HP
    AI_PACK_HUNTER, // +1 ATK per nearby ally
    AI_MERCHANT,    // Neutral shopkeeper
    AI_FINAL_BOSS   // The Stray King (Floor 30)
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
    int slowed_turns;
} Player;

typedef struct {
    int x, y, w, h;
} Room;

typedef struct {
    char date[32];
    int depth;
    int gold;
    bool won;
} HighScore;

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
extern char log_history[LOG_HISTORY_CAP][128];
extern int log_count;
extern int stairs_x, stairs_y;

// Mystery mappings for current run
extern MysteryEffect mystery_effects[MYSTERY_COUNT];
extern const char *MYSTERY_NAMES[MYSTERY_COUNT];

// Function Prototypes
void add_log(const char *fmt, ...);
void open_log_history(void);
void load_meta(void);
void save_meta(void);

void record_high_score(bool won);
void display_high_scores(void);

void init_mystery_items(void);
void generate_dungeon(void);
void compute_fov(void);
int get_floor_color_pair(int depth);

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
