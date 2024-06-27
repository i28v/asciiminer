#include "util.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>

#include <unistd.h>
#include <errno.h>

#define SCREEN_BUFFER_WIDTH 32
#define SCREEN_BUFFER_HEIGHT 32

#define CAMERA_WIDTH 32
#define CAMERA_HEIGHT 32

#define MINE_WIDTH 512
#define MINE_HEIGHT 512

#define SCRBUF_BLANK_CHAR ' '
#define SCRBUF_BLANK_COLOR 0

typedef struct {
    char c;
    unsigned char color;
} colorchar;

static colorchar screen_buffer[SCREEN_BUFFER_HEIGHT][SCREEN_BUFFER_WIDTH];

static void wrtscrb(int x, int y, char c, unsigned char color)
{
    screen_buffer[y][x].c = c;
    screen_buffer[y][x].color = color;
}

static void clrscrb()
{
    for(int y = 0; y < SCREEN_BUFFER_HEIGHT; y++) {
        for(int x = 0; x < SCREEN_BUFFER_WIDTH; x++) {
            wrtscrb(x, y, SCRBUF_BLANK_CHAR, SCRBUF_BLANK_COLOR);
        }
    }
}

static void prtscrb()
{
	int color;
    for(int y = 0; y < SCREEN_BUFFER_HEIGHT; y++) {
        for(int x = 0; x < SCREEN_BUFFER_WIDTH; x++) {
			color = screen_buffer[y][x].color;
			attron(COLOR_PAIR(color));
            addch(screen_buffer[y][x].c);
			attroff(COLOR_PAIR(color));
        }
		addch('\n');
    }
}

#define PLAYER_SYM '@'
#define DIRT_SYM '#'
#define ROCK_SYM 'O'
#define FALLING_ROCK_SYM '!'
#define ORE_SYM '%'
#define SUPPORT_SYM '|'
#define LADDER_SYM 'H'
#define EXIT_SHAFT_SYM 'H'
#define AIR_SYM '.'

#define MOVE_UP 'w'
#define MOVE_DOWN 's'
#define MOVE_LEFT 'a'
#define MOVE_RIGHT 'd'
#define ACTION_UP 'k'
#define ACTION_DOWN 'j'
#define ACTION_LEFT 'h'
#define ACTION_RIGHT 'l'
#define ACTION_CENTER '.'

#define NO_OP_KEY 'f'

#define MENU_UP 'k'
#define MENU_DOWN 'j'

#define MENU_SELECT '\n'

#define PLACE_LADDER_KEY 'z'
#define PLACE_SUPPORT_KEY 'x'
#define DIG_KEY 'c'

#define AUTO_DIG_KEY 'o'

#define USE_DYNAMITE_KEY 'v'

#define QUIT_KEY 'q'

#define VISIBLE ((type)128)

typedef enum {
    NO_DIRECTION = NONE,
    UP = DEFAULT,
    DOWN,
    LEFT,
    RIGHT
} directions;

typedef enum {
    AIR = DEFAULT,
    DIRT,
    EXIT_SHAFT,
    SUPPORT,
    LADDER,
    ROCK,
    FALLING_ROCK,
    COAL_BLOCK,
    IRON_BLOCK,
    COPPER_BLOCK,
    SILVER_BLOCK,
    GOLD_BLOCK,
    PLATINUM_BLOCK,
    TOTAL_BLOCKS
} block_types;

typedef enum {
    AIR_MINIMUM_TIER = NONE,
    DIRT_MINIMUM_TIER = DEFAULT,
    ROCK_MINIMUM_TIER = NONE,
    EXIT_SHAFT_MINIMUM_TIER = NONE,
    SUPPORT_MINIMUM_TIER = NONE,
    LADDER_MINIMUM_TIER = NONE,
    COAL_MINIMUM_TIER = 0,
    IRON_MINIMUM_TIER = 0,
    COPPER_MINIMUM_TIER = 1,
    SILVER_MINIMUM_TIER = 2,
    GOLD_MINIMUM_TIER = 3,
    PLATINUM_MINIMUM_TIER = 4
} block_minimum_tiers;

typedef enum {
    AIR_HEALTH = -1,
    DIRT_HEALTH = 10,
    ROCK_HEALTH = -1,
    EXIT_SHAFT_HEALTH = -1,
    SUPPORT_HEALTH = -1,
    LADDER_HEALTH = -1,
    COAL_HEALTH = 20,
    IRON_HEALTH = 30,
    COPPER_HEALTH = 50,
    SILVER_HEALTH = 75,
    GOLD_HEALTH = 90,
    PLATINUM_HEALTH = 120
} block_healths;


#if defined USING_WINDOWS

typedef enum {
    AIR_COLOR = 8,
    DIRT_COLOR = 6,
    ROCK_COLOR = 8,
    EXIT_SHAFT_COLOR = 4,
    COAL_COLOR = 8,
    IRON_COLOR = 14,
    COPPER_COLOR = 10,
    SILVER_COLOR = 15,
    GOLD_COLOR = 14,
    PLATINUM_COLOR = 11,
    LADDER_COLOR = 6,
    SUPPORT_COLOR = 6
} block_colors;

#else

typedef enum {
    AIR_COLOR = 242,
    DIRT_COLOR = 94,
    ROCK_COLOR = 183,
    EXIT_SHAFT_COLOR = 52,
    COAL_COLOR = 16,
    IRON_COLOR = 101,
    COPPER_COLOR = 202,
    SILVER_COLOR = 231,
    GOLD_COLOR = 226,
    PLATINUM_COLOR = 153,
    LADDER_COLOR = 94,
    SUPPORT_COLOR = 94
} block_colors;

#endif

typedef enum {
    NOT_ORE = NONE,
    COAL = DEFAULT,
    IRON,
    COPPER,
    SILVER,
    GOLD,
    PLATINUM,
    TOTAL_ORE
} ore_types;

typedef enum {
    COAL_PRICE = 32,
    IRON_PRICE = 64,
    COPPER_PRICE = 128,
    SILVER_PRICE = 256,
    GOLD_PRICE = 512,
    PLATINUM_PRICE = 1024
} ore_prices;

typedef enum {
    TIER1_BAG_PRICE = 2000,
    TIER2_BAG_PRICE = 4000,
    TIER3_BAG_PRICE = 8000,
    TIER4_BAG_PRICE = 16000
} bag_tier_prices;

typedef enum {
    ROCK_SPAWN_THRESHOLD = 1,
    COAL_SPAWN_THRESHOLD = 1,
    IRON_SPAWN_THRESHOLD = 1,
    COPPER_SPAWN_THRESHOLD = 60,
    SILVER_SPAWN_THRESHOLD = 160,
    GOLD_SPAWN_THRESHOLD = 160,
    PLATINUM_SPAWN_THRESHOLD = 384
} block_spawn_thresholds;

#define ROCK_CHANCE_MODIFIER 2

typedef enum {
    ROCK_CHANCE = 20,
    COAL_CHANCE = 10,
    IRON_CHANCE = 20,
    COPPER_CHANCE = 30,
    SILVER_CHANCE = 50,
    GOLD_CHANCE = 100,
    PLATINUM_CHANCE = 200
} block_chances;

typedef enum {
    RETURN_TO_MINE = DEFAULT,
    OPEN_SHOP,
    VIEW_STATS,
    EXIT_GAME,
    TOTAL_MENU_ACTIONS
} menu_actions;

typedef enum {
    UPGRADE_PICKAXE = DEFAULT,
    UPGRADE_BAG,
    BUY_COFFEE,
    BUY_DYNAMITE,
    BUY_SUPPORT,
    BUY_LADDER,
    BACK,
    TOTAL_SHOP_ACTIONS
} shop_actions;

typedef enum {
    TIER_1_PRICE = 4000,
    TIER_2_PRICE = 8000,
    TIER_3_PRICE = 16000,
    TIER_4_PRICE = 32000,
    TIER_5_PRICE = 64000,
    TIER_6_PRICE = 128000
} pickaxe_prices;

typedef enum {
    TIER_0_DAMAGE = 3,
    TIER_1_DAMAGE = 4,
    TIER_2_DAMAGE = 4,
    TIER_3_DAMAGE = 4,
    TIER_4_DAMAGE = 9,
    TIER_5_DAMAGE = 9,
    TIER_6_DAMAGE = 20
} pickaxe_damages;

typedef enum {
    COFFEE = DEFAULT,
    DYNAMITE,
    ITEM_SUPPORT,
    ITEM_LATTER
} item_types;

typedef enum {
    COFFEE_PRICE = 60,
    DYNAMITE_PRICE = 200,
    ITEM_SUPPORT_PRICE = 25,
    ITEM_LADDER_PRICE = 25
} item_prices;

typedef enum {
    STARTING_MAX_ORE = 16,
    STARTING_MAX_LADDERS = 16,
    STARTING_MAX_SUPPORTS = 16,
    STARTING_MAX_COFFEE = 4,
    STARTING_MAX_DYNAMITE = 4
} starting_max_items;

typedef enum {
    STARTING_COFFEE = 0,
    STARTING_DYNAMITE = 0,
    STARTING_SUPPORTS = 0,
    STARTING_LADDERS = 0
} starting_items;

typedef enum {
    MOVE_STAMINA_COST = 1,
    DIG_STAMINA_COST = 5
} action_stamina_costs;

typedef enum {
    NOT_RESCUED = NONE,
    OUT_OF_STAMINA = DEFAULT,
    CRUSHED_BY_ROCK,
    FALL
} rescue_reasons;

typedef enum {
    DIG = DEFAULT,
    BUILD_SUPPORT,
    BUILD_LADDER,
    USE_DYNAMITE
} player_actions;

typedef struct {
    type tier;
    char damage;
    int price;
} pickaxe;

typedef struct {
    type block_type;
    char health;
} block;

typedef struct {
    type block_type;
    type ore_type;
    type minimum_tier;
    char health;
    char symbol;
    unsigned char color;
    boolean solid_for_player;
    boolean solid_for_rocks;
} block_data;

static block_data blocks[TOTAL_BLOCKS] = {
    {AIR, NOT_ORE, AIR_MINIMUM_TIER, AIR_HEALTH, AIR_SYM, AIR_COLOR, False, False},
    {DIRT, NOT_ORE, DIRT_MINIMUM_TIER, DIRT_HEALTH, DIRT_SYM, DIRT_COLOR, True, True},
    {EXIT_SHAFT, NOT_ORE, EXIT_SHAFT_MINIMUM_TIER, EXIT_SHAFT_HEALTH, EXIT_SHAFT_SYM, EXIT_SHAFT_COLOR, False, True},
    {SUPPORT, NOT_ORE, SUPPORT_MINIMUM_TIER, SUPPORT_HEALTH, SUPPORT_SYM, SUPPORT_COLOR, False, True},
    {LADDER, NOT_ORE, LADDER_MINIMUM_TIER, LADDER_HEALTH, LADDER_SYM, LADDER_COLOR, False, False},
    {ROCK, NOT_ORE, ROCK_MINIMUM_TIER, ROCK_HEALTH, ROCK_SYM, ROCK_COLOR, True, True},
    {FALLING_ROCK, NOT_ORE, ROCK_MINIMUM_TIER, ROCK_HEALTH, FALLING_ROCK_SYM, ROCK_COLOR, True, True},
    {COAL_BLOCK, COAL, COAL_MINIMUM_TIER, COAL_HEALTH, ORE_SYM, COAL_COLOR, True, True},
    {IRON_BLOCK, IRON, IRON_MINIMUM_TIER, IRON_HEALTH, ORE_SYM,IRON_COLOR,  True, True},
    {COPPER_BLOCK, COPPER, COPPER_MINIMUM_TIER, COPPER_HEALTH, ORE_SYM, COPPER_COLOR, True, True},
    {SILVER_BLOCK, SILVER, SILVER_MINIMUM_TIER, SILVER_HEALTH, ORE_SYM,SILVER_COLOR,  True, True},
    {GOLD_BLOCK, GOLD, GOLD_MINIMUM_TIER, GOLD_HEALTH, ORE_SYM, GOLD_COLOR, True, True},
    {PLATINUM_BLOCK, PLATINUM, PLATINUM_MINIMUM_TIER, PLATINUM_HEALTH, ORE_SYM, PLATINUM_COLOR, True, True}
};

static int ore_price_data[TOTAL_ORE] = {
    COAL_PRICE,
    IRON_PRICE,
    COPPER_PRICE,
    SILVER_PRICE,
    GOLD_PRICE,
    PLATINUM_PRICE
};

static const char* menu_action_strs[TOTAL_MENU_ACTIONS] = {
    "Return to mine",
    "Open shop",
    "View stats",
    "Exit game"
};

static const char* ore_name_strs[TOTAL_ORE] = {
    "Coal",
    "Iron",
    "Copper",
    "Silver",
    "Gold",
    "Platinum"
};

static pickaxe pickaxe_data[7] = {
    {0, TIER_0_DAMAGE, 0},
    {1, TIER_1_DAMAGE, TIER_1_PRICE},
    {2, TIER_2_DAMAGE, TIER_2_PRICE},
    {3, TIER_3_DAMAGE, TIER_3_PRICE},
    {4, TIER_4_DAMAGE, TIER_4_PRICE},
    {5, TIER_5_DAMAGE, TIER_5_PRICE},
    {6, TIER_6_DAMAGE, TIER_6_PRICE}
};

static int bag_prices[5] = {
    0,
    TIER1_BAG_PRICE,
    TIER2_BAG_PRICE,
    TIER3_BAG_PRICE,
    TIER4_BAG_PRICE
};

static block mine[MINE_HEIGHT][MINE_WIDTH];

#define MAX_FALLING_ROCKS 32

static int falling_rocks[MAX_FALLING_ROCKS];
static int falling_rocks_top = 0;

static const int dirs[9][2] = {
    {-1, -1},
    {0,  -1},
    {1,  -1},
    {-1,  0},
    {0,   0},
    {1,   0},
    {-1,  1},
    {0,   1},
    {1,   1}
};

static const int max_stamina = 1000;
static const int starting_money = 0;

static const type max_pickaxe_tier = 6;
static const type max_bag_tier = 4;

static const int max_fall_distance = 6;

static const int player_start_x = 2;
static const int player_start_y = 1;

static const unsigned char player_color = 15;

static const int rock_fall_threshold = -2;

static const int sx = CAMERA_WIDTH + 2;
static const int status_y_offset = 1;

static int sy = 0;

static const int rescue_multiplier = 4;

static const int rescue_blinks = 5;
static const long rescue_blink_ms = 250;


static int stamina = max_stamina;

static int money = starting_money;

static type player_action = DEFAULT;
static type player_selected_structure = DEFAULT;
static type player_bag_tier = DEFAULT;

static int inv_ore = 0;
static int inv_ladders = STARTING_LADDERS;
static int inv_supports = STARTING_SUPPORTS;
static int inv_coffee = STARTING_COFFEE;
static int inv_dynamite = STARTING_DYNAMITE;

static int max_ore = STARTING_MAX_ORE;
static int max_supports = STARTING_MAX_SUPPORTS;
static int max_ladders = STARTING_MAX_LADDERS;
static int max_coffee = STARTING_MAX_COFFEE;
static int max_dynamite = STARTING_MAX_DYNAMITE;

static int inv_indv_ore[TOTAL_ORE];

static type player_pickaxe_tier = 0;

static int total_blocks_mined = 0;
static int total_ore_mined = 0;

static int total_money_earned = 0;
static int total_money_spent = 0;

static int coffee_bought = 0;
static int dynamite_bought = 0;
static int supports_bought = 0;
static int ladders_bought = 0;

static int coffee_used = 0;
static int dynamite_used = 0;

static int structures_placed = 0;
static int supports_placed = 0;
static int ladders_placed = 0;

static int times_rescued = 0;
static int money_spent_on_rescues = 0;
static int times_out_of_stamina = 0;
static int times_crushed_by_rock = 0;
static int times_fallen = 0;

static int total_indv_ore_mined[TOTAL_ORE];

static int player_x = player_start_x;
static int player_y = player_start_y;


static int player_scr_x = player_start_x;
static int player_scr_y = player_start_y;

static int camera_x = 0;
static int camera_y = 0;

static type selected_menu_action = DEFAULT;
static type selected_shop_action = DEFAULT;

static boolean game_running = True;
static boolean menu = False;
static boolean shop = False;

static boolean autodig = False;

static void reveal(int x, int y);
static void collapse_supports(int x, int y);

static void return_to_surface(type rescue_reason);

static void display_status();

static boolean use_coffee();

#define MERGE_XY(X,Y) ((int)(Y | (X << 16)))

#define X_MASK(N) ((int)((N & ((int)0xFFFF0000)) >> 16))
#define Y_MASK(N) ((int)(N & ((int)0x0000FFFF)))

static type ctdir(char dir)
{
    switch(dir) {
    case MOVE_UP:
    case ACTION_UP:
        return UP;
    case MOVE_DOWN:
    case ACTION_DOWN:
        return DOWN;
    case MOVE_LEFT:
    case ACTION_LEFT:
        return LEFT;
    case MOVE_RIGHT:
    case ACTION_RIGHT:
        return RIGHT;
    default:
        return NO_DIRECTION;
    }
}

static void move_dir(type dir, int* x, int* y)
{
    switch(dir) {
    case UP:
        if(y) (*y)--;
        break;
    case DOWN:
        if(y) (*y)++;
        break;
    case LEFT:
        if(x) (*x)--;
        break;
    case RIGHT:
        if(x) (*x)++;
        break;
    case NO_DIRECTION:
    default:
        break;
    }
}

static block* get_block(int x, int y)
{
    return &mine[y][x];
}

static int get_ore_price(type type)
{
    return ore_price_data[type];
}

static block_data* get_block_data(type type)
{
    return &blocks[type];
}

static block* put_block(int x, int y, type block_type)
{
    block* b = get_block(x, y);
    b->block_type = block_type;
    b->health = get_block_data(block_type)->health;
    return b;
}

static type get_block_type(block* b)
{
    return b->block_type & ~VISIBLE;
}

static void show_block(block* b)
{
    b->block_type |= VISIBLE;
}

static type get_ore_type(block* b)
{
    return get_block_data(get_block_type(b))->ore_type;
}

static type get_minimum_tier(block* b)
{
    return get_block_data(get_block_type(b))->minimum_tier;
}

static unsigned char get_color(block* b)
{
    return get_block_data(get_block_type(b))->color;
}

static char get_symbol(block* b)
{
    return get_block_data(get_block_type(b))->symbol;
}

static boolean is_visible(block* b)
{
    return (boolean)(b->block_type & VISIBLE);
}

static boolean is_solid_for_rocks(block* b)
{
    return get_block_data(get_block_type(b))->solid_for_rocks;
}

static boolean is_solid_for_player(block* b)
{
    return get_block_data(get_block_type(b))->solid_for_player;
}

static boolean above_block_non_solid(int x, int y)
{
    if(player_y > 0) {
        block* b = get_block(x, y);
        block* above = get_block(x, y - 1);
        return (is_solid_for_player(b)
                && !is_solid_for_player(above));
    }
    return False;
}

static pickaxe* get_pickaxe_data(type t)
{
    return &pickaxe_data[t];
}

static void set_falling_rock(int x, int y)
{
    put_block(x, y, FALLING_ROCK);
    reveal(x, y);
    falling_rocks[falling_rocks_top++] = MERGE_XY(x, y);
}

#define SAVE_INT_COUNT 37

static int* save_ints[SAVE_INT_COUNT] = {
    &player_x,
    &player_y,
    &player_scr_x,
    &player_scr_y,
    &camera_x,
    &camera_y,
    &money,
    &stamina,
    &inv_ore,
    &inv_supports,
    &inv_ladders,
    &inv_coffee,
    &inv_dynamite,
    &falling_rocks_top, 
    &max_ore,
    &max_supports,
    &max_ladders,
    &max_coffee,
    &max_dynamite,
    &total_blocks_mined,
    &total_ore_mined,
    &total_money_earned,
    &total_money_spent,
    &coffee_bought,
    &dynamite_bought,
    &supports_bought,
    &ladders_bought,
    &coffee_used,
    &dynamite_used,
    &structures_placed,
    &supports_placed,
    &ladders_placed,
    &times_rescued,
    &money_spent_on_rescues,
    &times_out_of_stamina,
    &times_crushed_by_rock,
    &times_fallen
};

#define ORE_RW_COUNT ((size_t)TOTAL_ORE)
#define FALLING_ROCKS_RW_COUNT ((size_t)MAX_FALLING_ROCKS)
#define MINE_RW_COUNT ((size_t)(MINE_WIDTH * MINE_HEIGHT))

static boolean save_game(const char* fn)
{
    FILE* f = fopen(fn, "wb");
    if(f == NULL) return False;
    for(int i = 0; i < SAVE_INT_COUNT; i++) {
        if(fwrite(save_ints[i], sizeof(int), 1, f) != 1) {
            fclose(f);
            return False;
        }
    }
    if(fwrite(&player_pickaxe_tier, sizeof(type), 1, f) != 1) {
        fclose(f);
        return False;
    }
    if(fwrite(&player_bag_tier, sizeof(type), 1, f) != 1) {
        fclose(f);
        return False;
    }
    if(fwrite(inv_indv_ore, sizeof(int), ORE_RW_COUNT, f) != ORE_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fwrite(total_indv_ore_mined, sizeof(int), ORE_RW_COUNT, f) != ORE_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fwrite(falling_rocks, sizeof(int), FALLING_ROCKS_RW_COUNT, f) != FALLING_ROCKS_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fwrite(mine, sizeof(block), MINE_RW_COUNT, f) != MINE_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fclose(f) == EOF) return False;
    return True;
}

static boolean load_game(const char* fn)
{
    FILE* f = fopen(fn, "rb");
    if(f == NULL) return False;
    for(int i = 0; i < SAVE_INT_COUNT; i++) {
        if(fread(save_ints[i], sizeof(int), 1, f) != 1) {
            fclose(f);
            return False;
        }
    }
    if(fread(&player_pickaxe_tier, sizeof(type), 1, f) != 1) {
        fclose(f);
        return False;
    }
    if(fread(&player_bag_tier, sizeof(type), 1, f) != 1) {
        fclose(f);
        return False;
    }
    if(fread(inv_indv_ore, sizeof(int), ORE_RW_COUNT, f) != ORE_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fread(total_indv_ore_mined, sizeof(int), ORE_RW_COUNT, f) != ORE_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fread(falling_rocks, sizeof(int), FALLING_ROCKS_RW_COUNT, f) != FALLING_ROCKS_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fread(mine, sizeof(block), MINE_RW_COUNT, f) != MINE_RW_COUNT) {
        fclose(f);
        return False;
    }
    if(fclose(f) == EOF) return False;
    return True;
}

static void reveal(int x, int y)
{
    int xdir, ydir, x_offset, y_offset;
    for(int i = 0; i < 9; i++) {
        xdir = dirs[i][0];
        ydir = dirs[i][1];
        x_offset = ((x + xdir > 0) && (x + xdir < MINE_WIDTH)) ? x + xdir : 0;
        y_offset = ((y + ydir > 0) && (y + ydir < MINE_HEIGHT)) ? y + ydir : 0;
        if(!(x_offset == 0 && y_offset == 0)) show_block(get_block(x_offset, y_offset));
    }
}

static void cam_render()
{
    for(int y = 0; y < CAMERA_HEIGHT; y++) {
        for(int x = 0; x < CAMERA_WIDTH; x++) {
            block* b = get_block(x + camera_x, y + camera_y);
            if(is_visible(b)) wrtscrb(x, y, get_symbol(b), get_color(b));
        }
    }
}



static void generate_mine()
{
    type bt;
    int d = 0;
    boolean rock = False;
    boolean coal = False;
    boolean iron = False;
    boolean copper = False;
    boolean silver = False;
    boolean gold = False;
    boolean platinum = False;
    for(int y = 0; y < MINE_HEIGHT; y++) {
        if(y == COAL_SPAWN_THRESHOLD) coal = True;
        if(y == IRON_SPAWN_THRESHOLD) iron = True;
        if(y == COPPER_SPAWN_THRESHOLD) {
            copper = True;
            d++;
        }
        if(y == SILVER_SPAWN_THRESHOLD) {
            silver = True;
            d++;
        }
        if(y == GOLD_SPAWN_THRESHOLD) {
            gold = True;
            d++;
        }
        if(y == PLATINUM_SPAWN_THRESHOLD) {
            platinum = True;
            d++;
        }
        for(int x = 0; x < MINE_WIDTH; x++) {
            if(x == 0 || y == 0 || x == MINE_WIDTH - 1 || y == MINE_HEIGHT - 1) {
                put_block(x, y, DIRT)->health = -1;
            } else {
                if(!(randint() % 2)) bt = DIRT;
                else { 
					switch(d) {
                    case 1:
                        rock = !(randint() % (ROCK_CHANCE - 4));
                        break;
                    case 2:
                        rock = !(randint() % (ROCK_CHANCE - 8));
                        break;
                    case 3:
                        rock = !(randint() % (ROCK_CHANCE - 12));
                        break;
                    case 4:
                        rock = !(randint() % (ROCK_CHANCE - 16));
                        break;
                    case 0:
                    default:
                        rock = !(randint() % ROCK_CHANCE);
                        break;
                    }
					if(!(randint() % PLATINUM_CHANCE) && platinum) bt = PLATINUM_BLOCK;
					else if(!(randint() % GOLD_CHANCE) && gold) bt = GOLD_BLOCK;
					else if(!(randint() % SILVER_CHANCE) && silver) bt = SILVER_BLOCK;
					else if(!(randint() % COPPER_CHANCE) && copper) bt = COPPER_BLOCK;
					else if(!(randint() % IRON_CHANCE) && iron) bt = IRON_BLOCK;
                    else if(!(randint() % COAL_CHANCE) && coal) bt = COAL_BLOCK;	
					else if(rock && !(x < 10 && y < 10)) bt = ROCK;
					else bt = DIRT; 
                }
                put_block(x, y, bt);
            }
        }
    }
}

static boolean build_structure(type structure, type direction)
{
    int x_offset = player_x;
    int y_offset = player_y;
    type s = structure;
    move_dir(direction, &x_offset, &y_offset);
    if(get_block_type(get_block(x_offset, y_offset)) == AIR) {
        switch(structure) {
        case SUPPORT:
            if(get_block_type(get_block(x_offset, y_offset + 1)) != AIR
               && inv_supports > 0) {
                inv_supports--;
                supports_placed++;
            }
            else s = NONE;
            break;
        case LADDER:
            if(inv_ladders > 0) {
                inv_ladders--;
                ladders_placed++;
            }
            else s = NONE;
            break;
        default:
            break;
        }
        if(s != NONE) {
            structures_placed++;
            put_block(x_offset, y_offset, s);
            reveal(x_offset, y_offset);
            return True;
        }
    }
    return False;
}

static void fall_rock(int x, int y, int index)
{
    boolean crushed = False;
    int x_offset = x;
    int y_offset = y;
    int orig_y = y;
    block* above_b = get_block(x_offset, y_offset - 1);
    if(get_block_type(above_b) == ROCK) {
        set_falling_rock(x_offset, y_offset - 1);
        above_b->health--;
    } else if(get_block_type(above_b) == SUPPORT) collapse_supports(x_offset, y_offset - 1);
    block* next_b;
    while(!is_solid_for_rocks(next_b = get_block(x_offset, y_offset + 1))) {
        if(get_block_type(next_b) == LADDER) show_block(put_block(x_offset, y_offset + 1, AIR));
        if(x_offset == player_x && y_offset + 1 == player_y) {
            if(inv_supports > 0) {
                build_structure(SUPPORT, NO_DIRECTION);
                break;
            } else crushed = True;
        }
        y_offset++;
    }
    for(int i = index; i < falling_rocks_top - 1; i++) falling_rocks[i] = falling_rocks[i+1];
    falling_rocks_top--;
    show_block(put_block(x_offset, orig_y, AIR));
    if(crushed) {
        int orig_player_x = player_x;
        int orig_player_y = player_y;
        show_block(put_block(orig_player_x, orig_player_y, ROCK));
        return_to_surface(CRUSHED_BY_ROCK);
        show_block(put_block(orig_player_x, orig_player_y, AIR));
    }
    put_block(x_offset, y_offset, ROCK);
    reveal(x_offset, y_offset);
}

static void fall_rocks()
{
    for(int i = 0; i < falling_rocks_top; i++) {
        int coords = falling_rocks[i];
        int fr_x = X_MASK(coords);
        int fr_y = Y_MASK(coords);
        block* fr = get_block(fr_x, fr_y);
        fr->health--;
        if(fr->health < rock_fall_threshold) {
            fall_rock(fr_x, fr_y, i);
        }
    }
}

static void collapse_supports(int x, int y)
{
    while(get_block_type(get_block(x, y)) == SUPPORT) {
        show_block(put_block(x, y, AIR));
        if(get_block_type(get_block(x, --y)) == ROCK) set_falling_rock(x, y);
    }
}

static void deplete_stamina(int amount)
{
    stamina -= amount;
    if(stamina <= 0) {
        if(!use_coffee()) return_to_surface(OUT_OF_STAMINA);
    }
}

static boolean dig(type direction)
{
    int x_offset = player_x;
    int y_offset = player_y;
    switch(direction) {
    case UP:
        if(player_y > 0) y_offset--;
        break;
    case DOWN:
        if(player_y < MINE_HEIGHT) y_offset++;
        break;
    case LEFT:
        if(player_x > 0) x_offset--;
        break;
    case RIGHT:
        if(player_x < MINE_WIDTH) x_offset++;
        break;
    case NO_DIRECTION:
    default:
        return False;
    }
    if(!(x_offset == player_x && y_offset == player_y)) {
        block* b = get_block(x_offset, y_offset);
        type ore_type = get_ore_type(b);
        if(!(ore_type != NOT_ORE && inv_ore == max_ore)) {
            if(get_block_type(b) != AIR && player_pickaxe_tier >= get_minimum_tier(b) && b->health > -1) {
                b->health -= get_pickaxe_data(player_pickaxe_tier)->damage;
                if(b->health <= 0) {
                    total_blocks_mined++;
                    if(ore_type != NOT_ORE) {
                        total_ore_mined++;
                        total_indv_ore_mined[ore_type]++;
                        if(inv_ore < max_ore) {
                            inv_ore++;
                            inv_indv_ore[ore_type]++;
                        }
                        show_block(put_block(x_offset, y_offset, DIRT));
                    } else {
                        put_block(x_offset, y_offset, AIR);
                        reveal(x_offset, y_offset);
                        if(y_offset - 1 > 0) {
                            block* upper_block = get_block(x_offset, y_offset - 1);
                            if(get_block_type(upper_block) == ROCK) set_falling_rock(x_offset, y_offset - 1);
                            else if(get_block_type(upper_block) == SUPPORT) collapse_supports(x_offset, y_offset - 1);
                        }
                    }
                }
                deplete_stamina(DIG_STAMINA_COST);
                return True;
            }
        }
    }
    return False;
}

static void movecam(type direction)
{
    switch(direction) {
    case UP:
        if(player_scr_y > 8) player_scr_y--;
        else if(camera_y > 0) camera_y--;
        else player_scr_y--;
        break;
    case DOWN:
        if(player_scr_y < CAMERA_HEIGHT - 8) player_scr_y++;
        else if(camera_y < MINE_HEIGHT - CAMERA_HEIGHT) camera_y++;
        else player_scr_y++;
        break;
    case RIGHT:
        if(player_scr_x < CAMERA_WIDTH - 8) player_scr_x++;
        else if(camera_x < MINE_WIDTH - CAMERA_WIDTH) camera_x++;
        else player_scr_x++;
        break;
    case LEFT:
        if(player_scr_x >  8) player_scr_x--;
        else if(camera_x > 0) camera_x--;
        else player_scr_x--;
        break;
    default:
        break;
    }
}

static boolean move_player(type direction, boolean forced)
{
    boolean moved = False;
    int x_offset = player_x;
    int y_offset = player_y;
    block* player_b = get_block(player_x, player_y);
    block* b;
    switch(direction) {
    case UP:
        b = get_block(x_offset, y_offset - 1);
        if(get_block_type(player_b) == LADDER && !is_solid_for_player(b)) {
            player_y--;
            movecam(UP);
            moved = True;
        }
        break;
    case DOWN:
        b = get_block(x_offset, y_offset + 1);
        if((!is_solid_for_player(b) && forced) || (get_block_type(b) == LADDER && !forced)) {
            player_y++;
            movecam(DOWN);
            moved = True;
        }
        break;
    case RIGHT:
        b = get_block(x_offset+1, y_offset);
        if(!is_solid_for_player(b)) {
            player_x++;
            movecam(RIGHT);
            moved = True;
        } else if(above_block_non_solid(x_offset + 1, y_offset) && player_y > 0) {
            player_x++;
            player_y--;
            movecam(RIGHT);
            movecam(UP);
            moved = True;
        } 
        break;
    case LEFT:
        b = get_block(x_offset-1, y_offset);
        if(!is_solid_for_player(b)) {
            player_x--;
            movecam(LEFT);
            moved = True;
        } else if(above_block_non_solid(x_offset - 1, y_offset) && player_y > 0) {
            player_x--;
            player_y--;
            movecam(LEFT);
            movecam(UP);
            moved = True;
        }
        break;
    default:
        break;
    }
    if(moved && !forced) deplete_stamina(MOVE_STAMINA_COST);
    return moved;
}

static int sell_ores()
{
    int amount = 0;
    for(int i = 0; i < TOTAL_ORE; i++) {
        amount += get_ore_price(i) * inv_indv_ore[i];
        inv_indv_ore[i] = 0;
    }
    inv_ore = 0;
    money += amount;
    total_money_earned += amount;
    return amount;
}

static void return_to_surface(type rescue_reason)
{
    if(rescue_reason != NOT_RESCUED) {
        for(int i = 0; i < rescue_blinks; i++) {
			erase();
            clrscrb();
            cam_render();
            block* b = get_block(player_x, player_y);
            wrtscrb(player_scr_x, player_scr_y, get_symbol(b), get_color(b));
            prtscrb();
			refresh();
            msleep(rescue_blink_ms);
			erase();
            clrscrb();
            cam_render();
            wrtscrb(player_scr_x, player_scr_y, PLAYER_SYM, player_color);
            prtscrb();
			refresh();
            msleep(rescue_blink_ms);
        }
        erase();
        int rescue_price = player_y * rescue_multiplier;
        money -= rescue_price;
        total_money_spent += rescue_price;
        money_spent_on_rescues += rescue_price;
        times_rescued++;
        switch(rescue_reason) {
        case OUT_OF_STAMINA:
            times_out_of_stamina++;
            printw("You ran out of stamina and had nothing to replenish it with");
            break;

        case CRUSHED_BY_ROCK:
            times_crushed_by_rock++;
            printw("You were crushed by a falling rock");
            break;
        case FALL:
            times_fallen++;
            printw("You fell down %d+ blocks", max_fall_distance);
            break;

        case NOT_RESCUED:
        default:
            break;
        }
        printw(" and had to be rescued for $%d\n", rescue_price);
        printw("Press enter to continue...");
        while(getch() != '\n');
        printw("\n\n");
    } else {
        erase();
    }
    stamina = max_stamina;
    player_x = 2;
    player_y = 1;
    player_scr_x = 2;
    player_scr_y = 1;
    camera_x = 0;
    camera_y = 0;
    printw("You return to the surface\n");
    if(inv_ore > 0) {
        printw("You sell your ore for $%d\n\n", sell_ores());
    } else printw("You have no ore to sell\n\n");
    printw("Press enter to continue...");
    while(getch() != '\n');
    erase();
    menu = True;
}

static boolean use_coffee()
{
    if(inv_coffee > 0) {
        stamina = max_stamina;
        inv_coffee--;
        coffee_used++;
        return True;
    }
    return False;
}

static boolean use_dynamite(type direction)
{
    if(inv_dynamite > 0) {
        int x_offset = player_x;
        int y_offset = player_y;
        move_dir(direction, &x_offset, &y_offset);
        if(get_block_type(get_block(x_offset, y_offset)) == ROCK) {
            inv_dynamite--;
            dynamite_used++;
            total_blocks_mined++;
            put_block(x_offset, y_offset, AIR);
            reveal(x_offset, y_offset);
            block* upper_block = get_block(x_offset, y_offset - 1);
            if(get_block_type(upper_block) == ROCK) {
                set_falling_rock(x_offset, y_offset - 1);
            } else if(get_block_type(upper_block) == SUPPORT) collapse_supports(x_offset, y_offset - 1);
            return True;
        }
    }
    return False;
}


static void game_update()
{
    boolean update = False;
    char ch = getch();
    type dir = ctdir(ch);
    switch(ch) {
    case NO_OP_KEY:
        update = True;
        break;
    case MOVE_UP:
    case MOVE_LEFT:
    case MOVE_DOWN:
    case MOVE_RIGHT:
        if(move_player(dir, False)) {
            if(player_x == 1 && player_y == 1) {
                return_to_surface(NOT_RESCUED);
            } else update = True;
        }
        else if(autodig) update = dig(dir);
        break;
    case ACTION_LEFT:
    case ACTION_DOWN:
    case ACTION_UP:
    case ACTION_RIGHT:
    case ACTION_CENTER:
        switch(player_action) {
        case DIG:
            update = dig(dir);
            break;
        case BUILD_SUPPORT:
        case BUILD_LADDER:
            update = build_structure(player_selected_structure, dir);
            break;
        case USE_DYNAMITE:
            update = use_dynamite(dir);
        default:
            break;
        } 
        break;
    case PLACE_LADDER_KEY:
        player_action = BUILD_LADDER;
        player_selected_structure = LADDER;
        break;
    case PLACE_SUPPORT_KEY:
        player_action = BUILD_SUPPORT;
        player_selected_structure = SUPPORT;
        break;
    case DIG_KEY:
        player_action = DIG;
        break;
    case AUTO_DIG_KEY:
        autodig = (!autodig) ? True : False;
        break;
    case USE_DYNAMITE_KEY:
        player_action = USE_DYNAMITE;
        break;
    case QUIT_KEY:
        game_running = False;
        break;
    default:
        break;
    }
    if(update) {
        int fall_distance = 0;
        while(get_block_type(get_block(player_x, player_y + 1)) == AIR) {
            move_player(DOWN, True);
            fall_distance++;
        }
        if(fall_distance > max_fall_distance) return_to_surface(FALL);
        fall_rocks();
    }
}

static void buy_item(int* item, int* max, int* total, int price)
{
    if(*item < *max && money >= price) {
        (*item)++;
        (*total)++;
        money -= price;
        total_money_spent += price;
    }
}

static void display_shop_upgrade(const char* str, int price, type tier, type max)
{
    printw("Upgrade %s", str);
    if(tier < max) printw(" to tier %d: $%d", tier + 1, price);
    else printw(": %s at max level", str);
}

static void display_shop_item(const char* str, int price, int inv, int max)
{
    printw("Buy %s: $%d %d/%d", str, price, inv, max);
}

static void game_menu()
{
    char ch;
    int next_pickaxe_price, next_bag_price;
	erase();
    printw("Your money: $%d\n\n", money);
    if(!shop) {
        printw("You are on the surface\n\n");
        printw("k and j to go up and down, enter to select\n\n");
        printw("Option:\n\n");
        for(int i = 0; i < TOTAL_MENU_ACTIONS; i++) {
            printw("%s", menu_action_strs[i]);
            if(i == selected_menu_action) addch('<');
            else addch(' ');
            addch('\n');
        }
        ch = getch();
        switch(ch) {
        case MENU_UP:
            if(selected_menu_action > 0) selected_menu_action--;
            break;
        case MENU_DOWN:
            if(selected_menu_action < TOTAL_MENU_ACTIONS - 1) selected_menu_action++;
            break;
        case MENU_SELECT:
            switch(selected_menu_action) {
            case RETURN_TO_MINE:
                erase();
                menu = False;
                break;
            case OPEN_SHOP:
                erase();
                shop = True;
                break;
            case VIEW_STATS:
                erase();
                erase();
                printw("Stats:\n\n");
                printw("Total blocks mined: %d\n", total_blocks_mined);
                printw("Total ore mined: %d\n", total_ore_mined);
                for(int i = 0; i < TOTAL_ORE; i++) {
                    printw("Total %s mined: %d\n", ore_name_strs[i], total_indv_ore_mined[i]);
                }
                printw("Current money: $%d\n", money);
                printw("Total money earned: $%d\n", total_money_earned);
                printw("Total money spent: $%d\n", total_money_spent);
                printw("Coffee bought: %d\n", coffee_bought);
                printw("Coffee used: %d\n", coffee_used);
                printw("Money spent on coffee: $%d\n", coffee_bought * COFFEE_PRICE);
                printw("Dynamite bought: %d\n", dynamite_bought);
                printw("Dynamite used: %d\n", dynamite_used);
                printw("Money spent on dynamite: $%d\n", dynamite_bought * DYNAMITE_PRICE);
                printw("Total structures placed: %d\n", structures_placed);
                printw("Supports bought: %d\n", supports_bought);
                printw("Supports placed: %d\n", supports_placed);
                printw("Money spent on supports: $%d\n", supports_bought * ITEM_SUPPORT_PRICE);
                printw("Ladders bought: %d\n", ladders_bought);
                printw("Ladders placed: %d\n", ladders_placed);
                printw("Money spent on ladders: $%d\n", ladders_bought * ITEM_LADDER_PRICE);
                printw("Times rescued: %d\n", times_rescued);
                printw("Money spent on being rescued: $%d\n", money_spent_on_rescues);
                printw("Times ran out of stamina: %d\n", times_out_of_stamina);
                printw("Times crushed by rock: %d\n", times_crushed_by_rock);
                printw("Times fallen: %d\n\n", times_fallen);
                printw("Press enter to continue...");
                while(getch() != '\n');
                erase();
                break;
            case EXIT_GAME:
                game_running = False;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    } else {
        printw("k and j to go up and down, o to select\n\n");
        for(int i = 0; i < TOTAL_SHOP_ACTIONS - 1; i++) {
            switch(i) {
            case UPGRADE_PICKAXE:
                next_pickaxe_price = (player_pickaxe_tier < max_pickaxe_tier) ? get_pickaxe_data(player_pickaxe_tier + 1)->price : 0;
                display_shop_upgrade("Pickaxe", next_pickaxe_price, player_pickaxe_tier, max_pickaxe_tier);
                break;
            case UPGRADE_BAG:
                next_bag_price = (player_bag_tier < max_bag_tier) ? bag_prices[player_bag_tier + 1] : 0; 
                display_shop_upgrade("Bag", next_bag_price, player_bag_tier, max_bag_tier);
                break;
            case BUY_COFFEE:
                display_shop_item("Coffee", COFFEE_PRICE, inv_coffee, max_coffee);
                break;
            case BUY_DYNAMITE:
                display_shop_item("Dynamite", DYNAMITE_PRICE, inv_dynamite, max_dynamite);
                break;
            case BUY_SUPPORT:
                display_shop_item("Support", ITEM_SUPPORT_PRICE, inv_supports, max_supports);
                break;
            case BUY_LADDER:
                display_shop_item("Ladder", ITEM_LADDER_PRICE, inv_ladders, max_ladders);
                break;
            default:
                break;
            }
            if(i == selected_shop_action) addch('<');
            else addch(' ');
            printw("\n");
        }
        printw("Back");
        if(selected_shop_action == BACK) addch('<');
        else addch(' ');
        ch = getch();
        switch(ch) {
        case MENU_UP:
            if(selected_shop_action > 0) selected_shop_action--;
            break;
        case MENU_DOWN:
            if(selected_shop_action < TOTAL_SHOP_ACTIONS - 1) selected_shop_action++;
            break;
        case MENU_SELECT:
            switch(selected_shop_action) {
            case UPGRADE_PICKAXE:
                if(player_pickaxe_tier < max_pickaxe_tier) {
                    pickaxe* next_p = get_pickaxe_data(player_pickaxe_tier + 1);
                    if(money >= next_p->price) {
                        money -= next_p->price;
                        total_money_spent += next_p->price;
                        player_pickaxe_tier++;
                    }
                }
                break;
            case UPGRADE_BAG:
                if(player_bag_tier < max_bag_tier) {
                    int next_bag_price = bag_prices[player_bag_tier + 1];
                    if(money >= next_bag_price) {
                        money -= next_bag_price;
                        total_money_spent += next_bag_price;
                        player_bag_tier++;
                        max_ore *= 2;
                        max_supports *= 2;
                        max_ladders *= 2;
                        max_coffee *= 2;
                        max_dynamite *= 2;
                    }
                }
                break;
            case BUY_COFFEE:
                buy_item(&inv_coffee, &max_coffee, &coffee_bought, COFFEE_PRICE);
                break;
            case BUY_DYNAMITE:
                buy_item(&inv_dynamite, &max_dynamite, &dynamite_bought, DYNAMITE_PRICE);
                break;
            case BUY_SUPPORT:
                buy_item(&inv_supports, &max_supports, &supports_bought, ITEM_SUPPORT_PRICE);
                break;
            case BUY_LADDER:
                buy_item(&inv_ladders, &max_ladders, &ladders_bought, ITEM_LADDER_PRICE);
                break;
            case BACK:
                erase();
                selected_shop_action = DEFAULT;
                shop = False;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

static void display_status(const char* str, int inc)
{
    printw("%s", str);
    sy += inc;
    move(sy, sx);
}

static void display_status_int(const char* str, int a, int inc) 
{
    printw("%s%d", str, a);
    sy += inc;
    move(sy, sx);
}

static void display_status_2ints(const char* str, char seperator, int a, int b, int inc)
{
    printw("%s%d%c%d", str, a, seperator, b);
    sy += inc;
    move(sy, sx);
}

static void display_status_ores()
{
    for(int i = 0; i < TOTAL_ORE; i++) {
        printw("%s: %d", ore_name_strs[i], inv_indv_ore[i]);
        move(++sy, sx);
    }
    move(++sy, sx);
}

static void display_status_action(const char* str, char key, type action, type structure, int inc)
{
    printw("%c - %s", key, str);
    if(player_action == action) addch('<');
    else addch(' ');
    sy += inc;
    move(sy, sx);
}

static void display_status_toggle(const char* str, char key, boolean toggle, int inc) 
{
    printw("%c - %s", key, str);
    if(toggle) printw("True ");
    else printw("False");
    sy += inc;
    move(sy, sx);
}

static void draw_status()
{
    sy = status_y_offset;
    move(sy, sx);
    display_status_int("Money: $", money, 2);
    display_status_2ints("Stamina: ", '/', stamina, max_stamina, 2);
    display_status_2ints("Pickaxe tier: ", '/', player_pickaxe_tier, max_pickaxe_tier, 2);
    display_status_2ints("Bag tier: ", '/', player_bag_tier, max_bag_tier, 2);
    display_status("Inventory:", 1);
    display_status("--------------------", 1);
    display_status_2ints("Total ore: ", '/', inv_ore, max_ore, 1);
    display_status_ores();
    display_status_2ints("Supports: ", '/', inv_supports, max_supports, 1);
    display_status_2ints("Ladders: ", '/', inv_ladders, max_ladders, 2);
    display_status_2ints("Coffee: ", '/', inv_coffee, max_coffee, 1);
    display_status_2ints("Dynamite: ", '/', inv_dynamite, max_dynamite, 1);
    display_status("--------------------", 1);
    display_status_2ints("Coordinates: ", ' ', player_x, player_y, 2);
    display_status("Actions:", 2);
    display_status_action("Dig", DIG_KEY, DIG, NONE, 1);
    display_status_action("Place support", PLACE_SUPPORT_KEY, BUILD_SUPPORT, SUPPORT, 1);
    display_status_action("Place ladder", PLACE_LADDER_KEY, BUILD_LADDER, LADDER, 1);
    display_status_action("Use dynamite", USE_DYNAMITE_KEY, USE_DYNAMITE, NONE, 2);
    display_status_toggle("Auto-dig: ", AUTO_DIG_KEY, autodig, 2);
    display_status("Other keys:", 2);
    display_status_action("Wait for rocks to fall", NO_OP_KEY, NONE, NONE, 1);
    display_status_action("Save and quit", QUIT_KEY, NONE, NONE, 1);
}

static void game_draw()
{
    clrscrb();
    cam_render();
    wrtscrb(player_scr_x, player_scr_y, PLAYER_SYM, player_color);
    prtscrb();
    draw_status();
}

static void game_init()
{
    memset(&mine, 0, sizeof(block) * (MINE_WIDTH * MINE_HEIGHT));
    memset(&falling_rocks, 0, sizeof(int) * MAX_FALLING_ROCKS);
    memset(&inv_indv_ore, 0, sizeof(int) * TOTAL_ORE);
    memset(&total_indv_ore_mined, 0, sizeof(int) * TOTAL_ORE);
    generate_mine();
    put_block(1, 1, EXIT_SHAFT);
    put_block(1, 2, DIRT)->health = -1;
    put_block(2, 2, DIRT)->health = -1;
    put_block(2, 1, AIR);
    put_block(3, 1, AIR);
    show_block(get_block(0, 0));
    for(int x = 1; x < 4; x++) {
        reveal(x, 1);
    }
}

static void sighandler(int sigtype)
{
    if(sigtype == SIGINT) game_running = False;
}

typedef enum {
    NO_ARG_EXCEPTION = NONE,
    PRINT_HELP = DEFAULT,
    ARG_INVALID,
    ARG2_NOT_INTEGER,
    ARG2_MORE_THAN_2_DIGITS,
    SCANF_ERROR,
    CANCELED_OVERWRITE,
    FILE_TO_LOAD_DOESNT_EXIST,
    FILE_TO_DELETE_DOESNT_EXIST,
    DELETED_FILE,
    CANCELED_DELETION
} argument_exceptions;

int main(int argc, char** argv)
{
    type arg_exc = NO_ARG_EXCEPTION;
    char* a1;
    char* a2;
    char savename[14] = "./save\0\0\0\0\0\0\0\0";
    char file_ext[5] = ".bin\0";
    char option = 'n';
    struct sigaction sa;
    boolean new_game = True;
    if(argc < 2) {
        arg_exc = PRINT_HELP;
        goto exception;
    }
    a1 = argv[1];
    if(argc == 2) {
        if(strcmp(a1, "--help") == 0 || strcmp(a1, "-h") == 0) {
            arg_exc = PRINT_HELP;
        } else arg_exc = ARG_INVALID;
        goto exception;
    }
    if(argc > 3 || !(strcmp(a1, "-n") == 0 || strcmp(a1, "-l") == 0 || strcmp(a1, "-d") == 0)) {
        arg_exc = ARG_INVALID;
        goto exception;
    }
    a2 = argv[2];
    if(!strisnum(a2)) {
        arg_exc = ARG2_NOT_INTEGER;
        goto exception;
    }
    if(strlen(a2) > 2) {
        arg_exc = ARG2_MORE_THAN_2_DIGITS;
        goto exception;
    }
    strcat(savename, a2);
    strcat(savename, file_ext);
    if(strcmp(a1, "-n") == 0) {
        if(file_exists(savename)) {
            printf("Save number %s already exists, overwrite (y/n)? ", a2);
            if(scanf("%c", &option) < 0) {
                arg_exc = SCANF_ERROR;
                goto exception;
            }
            if(option != 'y' && option != 'Y') {
                arg_exc = CANCELED_OVERWRITE;
                goto exception;
            }
            remove(savename);
        }
    } else if(strcmp(a1, "-l") == 0) {
        if(!file_exists(savename)) {
            arg_exc = FILE_TO_LOAD_DOESNT_EXIST;
            goto exception;
        }
        new_game = False;
    } else if(strcmp(a1, "-d") == 0) {
        if(!file_exists(savename)) {
            arg_exc = FILE_TO_DELETE_DOESNT_EXIST;
            goto exception;
        } else {
            printf("Confirm deletion of save number %s (y/n)? ", a2);
            if(scanf("%c", &option) < 0) {
                arg_exc = SCANF_ERROR;
                goto exception;
            }
            if(option == 'y' || option == 'Y') {
                remove(savename);
                arg_exc = DELETED_FILE;
            } else arg_exc = CANCELED_DELETION;
        }
    }
exception:
    switch(arg_exc) {
    case PRINT_HELP:
        printf("Usage: miner [OPTION] [SAVE FILE N]\n\n");
        printf("Options:\n\n");
        printf("-n - New game using save file N, if save file N does not exist it will be created,\n");
        printf("      if save file N exists there will be a prompt to overwrite it\n");
        printf("-l - Load save file N\n");
        printf("-d - Delete save file N\n");
        return 0;
    case ARG_INVALID:
        fprintf(stderr, "Error: Arguments are invalid\n");
        return -1;
    case ARG2_NOT_INTEGER:
        fprintf(stderr, "Error: Save file number \"%s\" is not an integer\n", a2);
        return -1;
    case ARG2_MORE_THAN_2_DIGITS:
        fprintf(stderr, "Error: Save file number %s is more than two digits\n", a2);
        return -1;
    case SCANF_ERROR:
        fprintf(stderr, "scanf() error\n");
        return -1;
    case FILE_TO_LOAD_DOESNT_EXIST:
    case FILE_TO_DELETE_DOESNT_EXIST:
        fprintf(stderr, "Error: Save file number %s doesn't exist\n", a2);
        return -1;
    case DELETED_FILE:
        printf("Deleted\n");
        return 0;
    case CANCELED_OVERWRITE:
    case CANCELED_DELETION:
        printf("Canceled\n");
        return 0;
    case NO_ARG_EXCEPTION:
    default:
        break;
    }
    set_seed((unsigned int)time(NULL));
    if(new_game) game_init();
    else {
        if(!load_game(savename)) {
            fprintf(stderr, "Error occured while reading from file %s", savename);
            return -1;
        }
    }
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sighandler;
    sigaction(SIGINT, &sa, 0);
	initscr();	
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	start_color();
	use_default_colors();
	init_pair(player_color, player_color, -1);
	for(int i = 0; i < TOTAL_BLOCKS; i++) {
		init_pair(blocks[i].color, blocks[i].color, -1);
	}
	erase();
    while(game_running) {
		move(0, 0);
        if(!menu) {
            game_draw();
            game_update();
        } else game_menu();
		refresh();
    }
	endwin();	
    if(!save_game(savename)) {
        fprintf(stderr, "Error occured while writing to file %s", savename);
        return -1;
    }
    return 0;
}
