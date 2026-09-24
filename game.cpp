// game.cpp
// Ties the systems together: loading levels, updating each frame, drawing.

#include "frost_hop.h"
#include <iostream>

using std::cout;
using std::endl;

static string level_file(int number)
{
    return "levels/level" + std::to_string(number) + ".txt";
}

void init_game(game_data &game)
{
    game.level = empty_level();
    game.player = create_player(point_at(0, 0));
    game.camera.x = 0;
    game.level_number = 1;
    load_current_level(game);
}

// Swaps in a fresh copy of the current level and puts Pip at its start
void load_current_level(game_data &game)
{
    free_level(game.level);
    game.level = load_level(level_file(game.level_number));
    place_player(game.player, game.level.spawn);
    game.camera.x = 0;
}

void update_game(game_data &game, double dt)
{
    player_data &player = game.player;
    level_data &level = game.level;

    collision_info info = update_player(player, level, dt);
    check_block_hits(player, level, info);
    collect_pickups(player, level);
    update_camera(game.camera, player, level);

    // Pits: for now falling in just restarts the level (lives come next)
    if (player.position.y > level.height * TILE_SIZE)
        load_current_level(game);

    if (level.touching(player_box(player), FLAG))
    {
        player.score += FLAG_POINTS;
        cout << "Flag reached! Score: " << player.score << endl;
        load_current_level(game);   // placeholder until the level complete screen exists
    }
}

static void draw_hud(const player_data &player)
{
    color text = rgb_color(20, 30, 50);
    fill_rectangle(rgba_color(255, 255, 255, 170), 0, 0, SCREEN_WIDTH, 22);
    draw_text("SCORE " + std::to_string(player.score), text, 10, 7);
    draw_text("COINS " + std::to_string(player.coins), text, 160, 7);
}

void draw_game(const game_data &game)
{
    clear_screen(rgb_color(170, 210, 240));
    draw_level(game.level, game.camera);
    draw_player(game.player, game.camera);
    draw_hud(game.player);
}
