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

// ---------- Player size and movement ----------
const double PLAYER_WIDTH = 24;
const double PLAYER_HEIGHT = 30;
const double GRAVITY = 1800;        // pixels per second, per second
const double MAX_FALL_SPEED = 600;  // pixels per second
const double RUN_SPEED = 220;
const double RUN_ACCEL = 1400;
const double RUN_DECEL = 1800;
const double JUMP_SPEED = 640;      // a full jump rises about 3.4 tiles
const double JUMP_CUT = 0.5;        // share of upward speed kept when jump is released early
const double COYOTE_TIME = 0.1;     // seconds Pip can still jump after running off a ledge

// ---------- Scoring ----------
const int COIN_POINTS = 100;
const int FLAG_POINTS = 1000;

enum tile_kind { EMPTY, GROUND, QUESTION, USED_BLOCK, COIN, FLAG };
enum player_state { IDLE, RUNNING, JUMPING, FALLING, DEAD };

struct player_data
{
    point_2d position;      // top-left corner, in world pixels
    vector_2d velocity;     // pixels per second
    player_state state;
    bool on_ground;
    int facing;             // 1 = right, -1 = left
    bool jump_held;         // true while the jump button is still held after a jump
    double coyote_timer;
    int coins;
    int score;
};

// The block of grid cells a rectangle covers
struct tile_range
{
    int left, right, top, bottom;
};

struct level_data
{
    tile_kind *tiles;       // width * height tiles, sized from the level file with new[]
    int width;
    int height;
    point_2d spawn;

    tile_kind tile_at(int col, int row) const;
    void set_tile(int col, int row, tile_kind kind);
    bool is_solid(int col, int row) const;
    bool solid_at(rectangle area) const;
    bool touching(rectangle area, tile_kind kind) const;
};

// The camera only stores how far the view has scrolled right, in world pixels
struct camera_data
{
    double x;
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
    level_data level;
    player_data player;
    camera_data camera;
    int level_number;
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
                                const level_data &level, double dt);

// player.cpp
player_data create_player(point_2d spawn);
void place_player(player_data &player, point_2d spawn);
rectangle player_box(const player_data &player);
void handle_input(player_data &player, double dt);
void apply_physics(player_data &player, double dt);
collision_info update_player(player_data &player, const level_data &level, double dt);
void draw_player(const player_data &player, const camera_data &cam);

// items.cpp
void check_block_hits(player_data &player, level_data &level, const collision_info &info);
void collect_pickups(player_data &player, level_data &level);

// camera.cpp
void update_camera(camera_data &cam, player_data &player, const level_data &level);

// game.cpp
void init_game(game_data &game);
void load_current_level(game_data &game);
void update_game(game_data &game, double dt);
void draw_game(const game_data &game);

#endif
