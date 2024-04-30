#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "util.h"

#define SCREEN_BUFFER_WIDTH 32
#define SCREEN_BUFFER_HEIGHT 32

#define CAMERA_WIDTH 32
#define CAMERA_HEIGHT 32

#define MINE_WIDTH 512
#define MINE_HEIGHT 512

#define SEGMENTS_X 4
#define SEGMENTS_Y 2

typedef struct {
    char c;
    int color;
} achar;

static achar screen_buffer[SCREEN_BUFFER_HEIGHT][SCREEN_BUFFER_WIDTH];

static inline void wrtscrb(int x, int y, char c, int color)
{
    screen_buffer[y][x].c = c;
    screen_buffer[y][x].color = color;
}

static void clrscrb(char c, int color)
{
    for(int y = 0; y < SCREEN_BUFFER_HEIGHT; y++) {
        for(int x = 0; x < SCREEN_BUFFER_WIDTH; x++) {
            wrtscrb(x, y, c, color);
        }
    }
}

static inline void prtscrb()
{
    for(int y = 0; y < SCREEN_BUFFER_WIDTH; y++) {
        for(int x = 0; x < SCREEN_BUFFER_HEIGHT; x++) {
            printf(COLOR "%c", screen_buffer[y][x].color, screen_buffer[y][x].c);
        }
        printf("\n");
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

typedef enum {
    AIR = 0,
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
    DIRT_HEALTH = 100,
    ROCK_HEALTH = -1,
    AIR_HEALTH = -1,
    COAL_HEALTH = 150,
    IRON_HEALTH = 200,
    COPPER_HEALTH = 350,
    SILVER_HEALTH = 500,
    GOLD_HEALTH = 1000,
    PLATINUM_HEALTH = 2000
} block_healths;

typedef enum {
    AIR_COLOR = 16,
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

typedef enum {
    DIRT_MINIMUM_TIER = 0,
    ROCK_MINIMUM_TIER = -1,
    AIR_MINIMUM_TIER = -1,
    COAL_MINIMUM_TIER = 0,
    IRON_MINIMUM_TIER = 0,
    COPPER_MINIMUM_TIER = 1,
    SILVER_MINIMUM_TIER = 2,
    GOLD_MINIMUM_TIER = 3,
    PLATINUM_MINIMUM_TIER = 4
} block_minimum_tiers;

typedef enum {
    NOT_ORE = -1,
    COAL = 0,
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
    ROCK_SPAWN_THRESHOLD = 1,
    COAL_SPAWN_THRESHOLD = 1,
    IRON_SPAWN_THRESHOLD = 1,
    COPPER_SPAWN_THRESHOLD = 60,
    SILVER_SPAWN_THRESHOLD = 160,
    GOLD_SPAWN_THRESHOLD = 160,
    PLATINUM_SPAWN_THRESHOLD = 384
} block_spawn_thresholds;

typedef enum {
    ROCK_CHANCE = 10,
    COAL_CHANCE = 20,
    IRON_CHANCE = 30,
    COPPER_CHANCE = 40,
    SILVER_CHANCE = 50,
    GOLD_CHANCE = 60,
    PLATINUM_CHANCE = 70
} block_chances;

typedef enum {
    TIER_1_PRICE = 500,
    TIER_2_PRICE = 4000,
    TIER_3_PRICE = 8000,
    TIER_4_PRICE = 6000,
    TIER_5_PRICE = 12000,
    TIER_6_PRICE = 24000
} pickaxe_prices;

typedef enum {
    TIER_0_DAMAGE = 25,
    TIER_1_DAMAGE = 50,
    TIER_2_DAMAGE = 100,
    TIER_3_DAMAGE = 200,
    TIER_4_DAMAGE = 400,
    TIER_5_DAMAGE = 800,
    TIER_6_DAMAGE = 1600
} pickaxe_damages;

typedef enum {
    COFFEE = 0,
    DYNAMITE,
    ITEM_SUPPORT,
    ITEM_LATTER
} item_types;

typedef enum {
    COFFEE_PRICE = 60,
    DYNAMITE_PRICE = 200,
    ITEM_SUPPORT_PRICE = 100,
    ITEM_LADDER_PRICE = 100
} item_prices;

typedef enum {
    NO_ACTION = 0,
    BUILD_SUPPORT,
    BUILD_LADDER
} player_actions;

typedef struct {
    int tier;
    int price;
    int damage;
} pickaxe;

typedef struct {
    int health;
    int minimum_tier;
    int x;
    int y;
    int color;
    int symbol;
    int block_type;
    int ore_type;
    BOOL visible;
} block;

static block block_data[TOTAL_BLOCKS] = {
    {AIR_HEALTH, AIR_MINIMUM_TIER, -1, -1, AIR_COLOR, ' ', AIR, NOT_ORE, FALSE},
    {DIRT_HEALTH, DIRT_MINIMUM_TIER, -1, -1, DIRT_COLOR, DIRT_SYM, DIRT, NOT_ORE, FALSE},
    {-1, -1, -1, -1, EXIT_SHAFT_COLOR, EXIT_SHAFT_SYM, EXIT_SHAFT, NOT_ORE, FALSE},
    {-1, -1, -1, -1, SUPPORT_COLOR, SUPPORT_SYM, SUPPORT, NOT_ORE, FALSE},
    {-1, -1, -1, -1, LADDER_COLOR, LADDER_SYM, LADDER, NOT_ORE, FALSE},
    {ROCK_HEALTH, ROCK_MINIMUM_TIER, -1, -1, ROCK_COLOR, ROCK_SYM, ROCK, NOT_ORE, FALSE},
    {-1, -1, -1, -1, ROCK_COLOR, FALLING_ROCK_SYM, FALLING_ROCK, NOT_ORE, FALSE},
    {COAL_HEALTH, COAL_MINIMUM_TIER, -1, -1, COAL_COLOR, ORE_SYM, COAL_BLOCK, COAL, FALSE},
    {IRON_HEALTH, IRON_MINIMUM_TIER, -1, -1, IRON_COLOR, ORE_SYM, IRON_BLOCK, IRON, FALSE},
    {COPPER_HEALTH, COPPER_MINIMUM_TIER, -1, -1, COPPER_COLOR, ORE_SYM, COPPER_BLOCK, COPPER, FALSE},
    {SILVER_HEALTH, SILVER_MINIMUM_TIER, -1, -1, SILVER_COLOR, ORE_SYM, SILVER_BLOCK, SILVER, FALSE},
    {GOLD_HEALTH, GOLD_MINIMUM_TIER, -1, -1, GOLD_COLOR, ORE_SYM, GOLD_BLOCK, GOLD, FALSE},
    {PLATINUM_HEALTH, PLATINUM_MINIMUM_TIER, -1, -1, PLATINUM_COLOR, ORE_SYM, PLATINUM_BLOCK, PLATINUM, FALSE},
};

static int ore_price_data[TOTAL_ORE] = {
    COAL_PRICE,
    IRON_PRICE,
    COPPER_PRICE,
    SILVER_PRICE,
    GOLD_PRICE,
    PLATINUM_PRICE
};

static pickaxe pickaxe_data[7] = {
    {0, 0, TIER_0_DAMAGE},
    {1, TIER_1_PRICE, TIER_1_DAMAGE},
    {2, TIER_2_PRICE, TIER_2_DAMAGE},
    {3, TIER_3_PRICE, TIER_3_DAMAGE},
    {4, TIER_4_PRICE, TIER_4_DAMAGE},
    {5, TIER_5_PRICE, TIER_5_DAMAGE},
    {6, TIER_6_PRICE, TIER_6_DAMAGE}
};

static block mine[MINE_HEIGHT][MINE_WIDTH];

static block* falling_rocks[8];
static int falling_rocks_top;

static const int player_max_stamina = 100;
static const int player_starting_pickaxe_tier = 0;
static const int player_starting_money = 100;

static const int player_start_x = 0;
static const int player_start_y = 0;

static const long input_delay_ms = 100;

static int player_stamina;
static int player_x, player_y;
static int player_lseg_x, player_lseg_y;
static int player_seg_x, player_seg_y;
static int player_money;
static int player_pickaxe_tier;
static int player_action;

static int player_scr_x, player_scr_y;

static int camera_x, camera_y;

static char pressed_key;

static BOOL game_running;

static BOOL no_update;

static void game_init();
static void game_update();
static void game_draw();

static void generate_mine();
static void get_direction(char dir, int* x, int* y);
static void reveal(int x, int y);

static inline void move_dir(char dir, int* x, int* y)
{
    switch(dir) {
    case 'k':
    case 'w':
        if(y) (*y)--;
        break;
    case 'j':
    case 's':
        if(y) (*y)++;
        break;
    case 'h':
    case 'a':
        if(x) (*x)--;
        break;
    case 'l':
    case 'd':
        if(x) (*x)++;
        break;
    default:
        break;
    }
}

static inline block* get_block(int x, int y)
{
    return &mine[y][x];
}

static inline int get_ore_price(int type)
{
    return ore_price_data[type];
}

static inline block* get_block_data(int type)
{
    return &block_data[type];
}

static inline block* get_falling_rock(int index)
{
    return falling_rocks[index];
}

static inline BOOL is_solid(block* b)
{
    return (b->block_type != AIR && b->block_type != LADDER);
}

static inline BOOL air_above_block(block* b)
{
    return (b->block_type != AIR
            && get_block(b->x, b->y - 1)->block_type == AIR);
}

static inline pickaxe* get_pickaxe_data(int type)
{
    return &pickaxe_data[type];
}

static inline block* put_block(int x, int y, int block_type)
{
    block* b = get_block(x, y);
    block* bd = get_block_data(block_type);
    memcpy(b, bd, sizeof(block));
    b->x = x;
    b->y = y;
    return b;
}

static inline void set_falling_rock(block* b)
{
    put_block(b->x, b->y, FALLING_ROCK);
    reveal(b->x, b->y);
    falling_rocks[falling_rocks_top++] = b;
}

static void game_init()
{
    clear_screen();
    reset_cursor();
    memset(&mine, 0, sizeof(block) * (MINE_WIDTH * MINE_HEIGHT));
    generate_mine();
    put_block(0, 0, AIR);
    put_block(1, 0, AIR);
    put_block(2, 0, AIR);
    for(int x = 0; x < 4; x++) {
        get_block(x, 1)->visible = TRUE;
    }
    get_block(3, 0)->visible = TRUE;
    player_stamina = player_max_stamina;
    player_pickaxe_tier = 6;
    player_x = player_start_x;
    player_y = player_start_y;
    player_scr_x = player_start_x;
    player_scr_y = player_start_y;
    player_action = NO_ACTION;
    camera_x = 0;
    camera_y = 0;
    falling_rocks_top = 0;
    game_running = TRUE;
    no_update = FALSE;
}

static void game_over()
{
    clear_screen();
    reset_cursor();
    printf("Game over!\nPress any key...\n");
    getchar();
}

static void cam_render()
{
    for(int y = 0; y < CAMERA_HEIGHT; y++) {
        for(int x = 0; x < CAMERA_WIDTH; x++) {
            block* b = get_block(x + camera_x, y + camera_y);
            if(b->visible) wrtscrb(x, y, b->symbol, b->color);
        }
    }
}

static void generate_mine()
{
    int b, n;
    BOOL rock = FALSE;
    BOOL coal = TRUE;
    BOOL iron = FALSE;
    BOOL copper = FALSE;
    BOOL silver = FALSE;
    BOOL gold = FALSE;
    BOOL platinum = FALSE;
    for(int y = 0; y < MINE_HEIGHT; y++) {
        if(y == ROCK_SPAWN_THRESHOLD) rock = TRUE;
        if(y == COAL_SPAWN_THRESHOLD) coal = TRUE;
        if(y == IRON_SPAWN_THRESHOLD) iron = TRUE;
        if(y == COPPER_SPAWN_THRESHOLD) copper = TRUE;
        if(y == SILVER_SPAWN_THRESHOLD) silver = TRUE;
        if(y == GOLD_SPAWN_THRESHOLD) gold = TRUE;
        if(y == PLATINUM_SPAWN_THRESHOLD) platinum = TRUE;
        for(int x = 0; x < MINE_WIDTH; x++) {
            b = 0;
            if(x == 0 || y == 0 || x == MINE_WIDTH - 1 || y == MINE_HEIGHT - 1) {
                block* bd = put_block(x, y, DIRT);
                bd->health = -1;
            } else {
                n = randint() % 2;
                if(n == 1) b = DIRT;
                else {
                    n = randint() % ROCK_CHANCE;
                    if(!n && rock) b = ROCK;
                    else {
                        n = randint() % COAL_CHANCE;
                        if(!n && coal) b = COAL_BLOCK;
                        else {
                            n = randint() % IRON_CHANCE;
                            if(!n && iron) b = IRON_BLOCK;
                            else {
                                n = randint() % COPPER_CHANCE;
                                if(!n && copper) b = COPPER_BLOCK;
                                else {
                                    n = randint() % SILVER_CHANCE;
                                    if(!n && silver) b = SILVER_BLOCK;
                                    else {
                                        n = randint() % GOLD_CHANCE;
                                        if(!n && gold) b = GOLD_BLOCK;
                                        else {
                                            n = randint() % PLATINUM_CHANCE;
                                            if(!n && platinum) b = PLATINUM_BLOCK;
                                            else b = DIRT;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                put_block(x, y, b);
            }
        }
    }
}

static void fall_rock(int index)
{
    block* b = falling_rocks[index];
    int x_offset = b->x;
    int y_offset = b->y;
    while(!is_solid(get_block(x_offset, y_offset + 1))) {
        if(get_block(x_offset, y_offset + 1)->block_type == LADDER) put_block(x_offset, y_offset + 1, AIR);
        y_offset++;
    }
    put_block(b->x,b->y, AIR);
    put_block(x_offset, y_offset, ROCK);
    reveal(x_offset, y_offset);
    for(int i = index; i < falling_rocks_top - 1; i++) falling_rocks[i] = falling_rocks[i + 1];
    falling_rocks_top--;
    if(player_x == x_offset && player_y == y_offset) game_running = FALSE;
}

static void reveal(int x, int y)
{
    get_block(x, y)->visible = TRUE;
    int dirs[8][2] = {
        {-1, -1},
        {0,  -1},
        {1,  -1},
        {-1,  0},
        {1,   0},
        {-1,  1},
        {0,   1},
        {1,   1}
    };
    for(int i = 0; i < 8; i++) {
        int xdir = dirs[i][0];
        int ydir = dirs[i][1];
        int xvisoffset = ((x + xdir > 0) && (x + xdir < MINE_WIDTH)) ? x + xdir : 0;
        int yvisoffset = ((y + ydir > 0) && (y + ydir < MINE_HEIGHT)) ? y + ydir : 0;
        block* b = get_block(xvisoffset, yvisoffset);
        b->visible = TRUE;
    }
}

static void dig(unsigned char direction)
{
    int x_offset = player_x;
    int y_offset = player_y;
    block* b;
    switch(direction) {
    case 'k':
        if(player_y > 0) y_offset--;
        break;
    case 'j':
        if(player_y < MINE_HEIGHT) y_offset++;
        break;
    case 'h':
        if(player_x > 0) x_offset--;
        break;
    case 'l':
        if(player_x < MINE_WIDTH) x_offset++;
        break;
    default:
        break;
    }
    if(!(x_offset == player_x && y_offset == player_y)) {
        b = get_block(x_offset, y_offset);
        if(b->block_type != AIR && player_pickaxe_tier >= b->minimum_tier && b->health > -1) {
            pickaxe* p = get_pickaxe_data(player_pickaxe_tier);
            b->health -= p->damage;
            if(b->health <= 0) {
                if(b->ore_type != NOT_ORE) player_money += get_ore_price(b->ore_type);
                put_block(x_offset, y_offset, AIR);
                reveal(x_offset, y_offset);
                if(y_offset - 1 > 0) {
                    block* upper_block = get_block(x_offset, y_offset - 1);
                    if(upper_block->block_type == ROCK) set_falling_rock(upper_block);
                }
            }
        }
    }
}

static void scroll(char direction)
{
    switch(direction) {
    case 'w':
        if(player_scr_y > 8) player_scr_y--;
        else if(camera_y > 0) camera_y--;
        else player_scr_y--;
        break;
    case 's':
        if(player_scr_y < CAMERA_HEIGHT - 8) player_scr_y++;
        else if(camera_y < MINE_HEIGHT - CAMERA_HEIGHT) camera_y++;
        else player_scr_y++;
        break;
    case 'd':
        if(player_scr_x < CAMERA_WIDTH - 8) player_scr_x++;
        else if(camera_x < MINE_WIDTH - CAMERA_WIDTH) camera_x++;
        else player_scr_x++;
        break;
    case 'a':
        if(player_scr_x >  8) player_scr_x--;
        else if(camera_x > 0) camera_x--;
        else player_scr_x--;
        break;
    default:
        break;
    }
}

static void move_player(char direction)
{
    int x_offs = player_x;
    int y_offs = player_y;
    block* b;
    switch(direction) {
    case 'w':
        b = get_block(x_offs, y_offs - 1);
        if(!is_solid(b)) {
            player_y--;
            scroll('w');
        }
        break;
    case 's':
        b = get_block(x_offs, y_offs + 1);
        if(!is_solid(b)) {
            player_y++;
            scroll('s');
        }
        break;
    case 'd':
        b = get_block(x_offs + 1, y_offs);
        if(!is_solid(b)) {
            player_x++;
            scroll('d');
        } else if(air_above_block(b) && player_y > 0) {
            player_x++;
            scroll('d');
            player_y--;
            scroll('w');
        }
        break;
    case 'a':
        b = get_block(x_offs - 1, y_offs);
        if(!is_solid(b)) {
            player_x--;
            scroll('a');
        } else if(air_above_block(b) && player_y > 0) {
            player_x--;
            scroll('a');
            player_y--;
            scroll('w');
        }
        break;
    default:
        break;
    }
}

static void build_ladder(char direction)
{
    int x_offset = player_x;
    int y_offset = player_y;
    move_dir(direction, &x_offset, &y_offset);
    if(get_block(x_offset, y_offset)->block_type == AIR) {
        put_block(x_offset, y_offset, LADDER);
        reveal(x_offset, y_offset);
    }
}

static void game_update()
{
    char ch = getchar();
    switch(ch) {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
        if(player_action == NO_ACTION) dig(ch);
        else if(player_action == BUILD_LADDER) build_ladder(ch);
        no_update = FALSE;
        break;
    case 'w':
    case 'a':
    case 's':
    case 'd':
        move_player(ch);
        no_update = FALSE;
        break;
    case 'z':
        player_action = BUILD_LADDER;
        no_update = TRUE;
        break;
    case 'x':
        player_action = BUILD_SUPPORT;
        no_update = TRUE;
        break;
    case 'c':
        player_action = NO_ACTION;
        no_update = TRUE;
        break;
    case 'q':
        game_running = FALSE;
        break;
    default:
        break;
    }
    if(!no_update) {
        while(get_block(player_x, player_y + 1)->block_type == AIR) move_player('s');
        for(int i = 0; i < falling_rocks_top; i++) {
            block* fr = get_falling_rock(i);
            fr->health--;
            if(fr->health < -2) {
                fall_rock(i);
            }
        }
    }
}

static void game_draw()
{
    reset_cursor();
    clrscrb(' ', 0);
    cam_render();
    wrtscrb(player_scr_x, player_scr_y, PLAYER_SYM, 15);
    prtscrb();
    reset_color();
    printf("%d\n\n", player_money);
    printf("\n%d %d\n", player_x, player_y);
    gotoxy(34, 25);
    printf("test");
}

int main(void)
{
    hide_cursor();
    set_seed((unsigned int)time(NULL));
    modify_terminal();
    game_init();
    while(game_running == TRUE) {
        game_draw();
        game_update();
    }
    game_over();
    restore_old_terminal();
    show_cursor();
    return 0;
}
