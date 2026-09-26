// frost_hop.h
// Shared constants, types and function declarations for Frost Hop.

#ifndef FROST_HOP_H
#define FROST_HOP_H

#include "splashkit.h"
#include <string>

using std::string;

// ---------- Window and world ----------
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 32;
const double MAX_DT = 1.0 / 30.0;   // cap on frame time, so a lag spike cannot push Pip through a wall
const int LEVEL_COUNT = 3;

// ---------- Player size and movement ----------
const double PLAYER_WIDTH = 24;
const double PLAYER_HEIGHT = 30;
const double GRAVITY = 1800;        // pixels per second, per second
const double MAX_FALL_SPEED = 600;  // pixels per second
const double RUN_SPEED = 220;
const double RUN_ACCEL = 1400;
const double RUN_DECEL = 1800;
const double JUMP_SPEED = 640;      // a full jump rises about 3.4 tiles, so 4-tile ledges need a step
const double JUMP_CUT = 0.5;        // share of upward speed kept when jump is released early
const double COYOTE_TIME = 0.1;     // seconds Pip can still jump after running off a ledge
const double STOMP_BOUNCE = 420;    // upward speed after landing on a Grumble
const double INVINCIBLE_TIME = 1.0; // flashing time after losing the Frost Flower
const double DEATH_PAUSE = 1.0;     // seconds before the level restarts
const int START_LIVES = 3;

// ---------- Grumbles ----------
const double GRUMBLE_WIDTH = 28;
const double GRUMBLE_HEIGHT = 30;   // a frozen Grumble adds almost a tile to Pip's reach
const double GRUMBLE_SPEED = 60;
const double FREEZE_TIME = 5.0;
const double THAW_WARNING = 1.0;    // the ice flashes for this long before it thaws
const double SQUASH_TIME = 0.4;     // how long a squashed Grumble stays on screen
const double ACTIVE_MARGIN = 64;    // Grumbles this far outside the view still update

// ---------- Snowballs ----------
const int MAX_SNOWBALLS = 2;
const double SNOWBALL_SIZE = 10;
const double SNOWBALL_SPEED = 380;
const double SNOWBALL_LIFT = 120;   // small upward toss, so throws arc down into dips
const double SNOWBALL_GRAVITY = 900;

// ---------- Scoring ----------
const int COIN_POINTS = 100;
const int FREEZE_POINTS = 50;
const int STOMP_POINTS = 200;
const int FLOWER_POINTS = 500;
const int FLAG_POINTS = 1000;

enum tile_kind { EMPTY, GROUND, QUESTION, USED_BLOCK, COIN, SPIKE, FLAG, FLOWER };
enum power_state { SMALL, FROST };
enum player_state { IDLE, RUNNING, JUMPING, FALLING, DEAD };
enum enemy_state { WALKING, FROZEN, SQUASHED, GONE };
enum game_screen { TITLE_SCREEN, PLAYING, LEVEL_COMPLETE, GAME_OVER };

struct player_data
{
    point_2d position;      // top-left corner, in world pixels
    vector_2d velocity;     // pixels per second
    player_state state;
    power_state power;
    bool on_ground;
    int facing;             // 1 = right, -1 = left
    bool jump_held;         // true while the jump button is still held after a jump
    double coyote_timer;
    double invincible_timer;
    int lives;
    int coins;
    int score;
};

struct enemy
{
    point_2d position;
    vector_2d velocity;
    enemy_state state;
    int direction;          // 1 = right, -1 = left
    double timer;           // time left in the current state (frozen or squashed)
};

struct snowball
{
    point_2d position;
    vector_2d velocity;
    bool active;
};

// The block of grid cells a rectangle covers
struct tile_range
{
    int left, right, top, bottom;
};

struct level_data
{
    tile_kind *tiles;       // width * height tiles, sized from the level file with new[]
    bool *has_flower;       // same size: marks question blocks holding a Frost Flower
    enemy *enemies;         // one per 'g' in the file, sized with new[]
    int enemy_count;
    int width;
    int height;
    point_2d spawn;

    tile_kind tile_at(int col, int row) const;
    void set_tile(int col, int row, tile_kind kind);
    bool is_solid(int col, int row) const;
    bool solid_at(rectangle area, int ignore_enemy = -1) const;
    bool touching(rectangle area, tile_kind kind) const;
};

// The camera only stores how far the view has scrolled right, in world pixels
struct camera_data
{
    double x;
};

// A fixed set of snowballs, reused instead of created and destroyed
struct snowball_pool
{
    snowball items[MAX_SNOWBALLS];

    void clear();
    bool throw_from(point_2d start, int facing);
    void update(double dt, level_data &level, const camera_data &cam, player_data &player);
};

// What a move ran into
struct collision_info
{
    bool on_ground;
    bool hit_wall;
    bool hit_ceiling;
    int ceiling_col;
    int ceiling_row;
};

struct game_data
{
    game_screen screen;
    level_data level;
    player_data player;
    camera_data camera;
    snowball_pool snowballs;
    int level_number;
    double death_timer;
};

// level.cpp
tile_range tiles_under(const rectangle &area);
level_data empty_level();
level_data load_level(const string &filename);
void free_level(level_data &level);
void draw_level(const level_data &level, const camera_data &cam);

// physics.cpp
bool overlaps(const rectangle &a, const rectangle &b);
collision_info move_and_collide(point_2d &position, vector_2d &velocity, double width, double height,
                                const level_data &level, double dt, int ignore_enemy = -1);

// player.cpp
player_data create_player(point_2d spawn);
void place_player(player_data &player, point_2d spawn);
rectangle player_box(const player_data &player);
void handle_input(player_data &player, snowball_pool &snowballs, double dt);
void apply_physics(player_data &player, double dt);
collision_info update_player(player_data &player, const level_data &level, snowball_pool &snowballs, double dt);
void damage_player(player_data &player);
void kill_player(player_data &player);
void draw_player(const player_data &player, const camera_data &cam);

// items.cpp
void check_block_hits(player_data &player, level_data &level, const collision_info &info);
void collect_pickups(player_data &player, level_data &level);

// camera.cpp
void update_camera(camera_data &cam, player_data &player, const level_data &level);
bool in_active_range(double x, const camera_data &cam);

// enemies.cpp
enemy create_grumble(point_2d position);
rectangle enemy_box(const enemy &grumble);
void freeze_enemy(enemy &grumble, player_data &player);
void update_enemies(level_data &level, player_data &player, const camera_data &cam, double dt);
void check_enemy_contact(player_data &player, level_data &level, double old_bottom);
void draw_grumble(double x, double y, int direction);
void draw_enemies(const level_data &level, const camera_data &cam);

// snowballs.cpp
void draw_snowballs(const snowball_pool &pool, const camera_data &cam);

// game.cpp
void init_game(game_data &game);
void start_new_game(game_data &game);
void load_current_level(game_data &game);
void update_game(game_data &game, double dt);
void draw_game(const game_data &game);

#endif
