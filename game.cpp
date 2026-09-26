// game.cpp
// Ties the systems together: the screen state machine, loading levels,
// updating each frame and drawing.

#include "frost_hop.h"

static string level_file(int number)
{
    return "levels/level" + std::to_string(number) + ".txt";
}

void init_game(game_data &game)
{
    game.screen = TITLE_SCREEN;
    game.level = empty_level();
    game.player = create_player(point_at(0, 0));
    game.camera.x = 0;
    game.snowballs.clear();
    game.level_number = 1;
    game.death_timer = 0;
}

void start_new_game(game_data &game)
{
    game.player = create_player(point_at(0, 0));
    game.level_number = 1;
    load_current_level(game);
    game.screen = PLAYING;
}

// Swaps in a fresh copy of the current level and puts Pip at its start
void load_current_level(game_data &game)
{
    free_level(game.level);
    game.level = load_level(level_file(game.level_number));
    place_player(game.player, game.level.spawn);
    game.camera.x = 0;
    game.snowballs.clear();
    game.death_timer = 0;
}

// After a short pause, spend a life and restart the level, or end the game
static void update_death(game_data &game, double dt)
{
    game.death_timer -= dt;
    if (game.death_timer > 0)
        return;

    game.player.lives--;
    if (game.player.lives <= 0)
        game.screen = GAME_OVER;
    else
    {
        game.player.power = SMALL;
        load_current_level(game);
    }
}

// Pits and spikes are instant hazards, with or without the Frost Flower
static void check_hazards(player_data &player, const level_data &level)
{
    rectangle feel = rectangle_from(player.position.x - 1, player.position.y - 1,
                                    PLAYER_WIDTH + 2, PLAYER_HEIGHT + 2);
    if (player.position.y > level.height * TILE_SIZE || level.touching(feel, SPIKE))
        kill_player(player);
}

static void update_playing(game_data &game, double dt)
{
    player_data &player = game.player;
    level_data &level = game.level;

    if (player.state == DEAD)
    {
        update_death(game, dt);
        return;
    }

    double old_bottom = player.position.y + PLAYER_HEIGHT;
    collision_info info = update_player(player, level, game.snowballs, dt);
    check_block_hits(player, level, info);
    collect_pickups(player, level);
    update_camera(game.camera, player, level);
    update_enemies(level, player, game.camera, dt);
    game.snowballs.update(dt, level, game.camera, player);
    check_enemy_contact(player, level, old_bottom);
    check_hazards(player, level);

    if (player.state == DEAD)
        game.death_timer = DEATH_PAUSE;
    else if (level.touching(player_box(player), FLAG))
    {
        player.score += FLAG_POINTS;
        game.screen = LEVEL_COMPLETE;
    }
}

// Enter moves on to the next level, or back to the title after the last one
static void finish_level(game_data &game)
{
    if (game.level_number < LEVEL_COUNT)
    {
        game.level_number++;
        load_current_level(game);
        game.screen = PLAYING;
    }
    else
        game.screen = TITLE_SCREEN;
}

void update_game(game_data &game, double dt)
{
    bool enter = key_typed(RETURN_KEY);
    switch (game.screen)
    {
        case TITLE_SCREEN:
            if (enter) start_new_game(game);
            break;
        case PLAYING:
            update_playing(game, dt);
            break;
        case LEVEL_COMPLETE:
            if (enter) finish_level(game);
            break;
        case GAME_OVER:
            if (enter) game.screen = TITLE_SCREEN;
            break;
    }
}

// ---------- Drawing ----------

static void draw_hud(const game_data &game)
{
    const player_data &player = game.player;
    color text = rgb_color(20, 30, 50);
    fill_rectangle(rgba_color(255, 255, 255, 170), 0, 0, SCREEN_WIDTH, 22);
    draw_text("SCORE " + std::to_string(player.score), text, 10, 7);
    draw_text("COINS " + std::to_string(player.coins), text, 160, 7);
    draw_text("LIVES " + std::to_string(player.lives), text, 290, 7);
    draw_text("LEVEL " + std::to_string(game.level_number), text, 410, 7);
    if (player.power == FROST)
        draw_text("FROST (J to throw)", rgb_color(40, 100, 200), 530, 7);
}

static void draw_panel(const string &title, const string &line1, const string &line2)
{
    fill_rectangle(rgba_color(20, 30, 50, 210), 200, 170, 400, 120);
    draw_text(title, rgb_color(255, 255, 255), 230, 195);
    draw_text(line1, rgb_color(200, 225, 255), 230, 225);
    draw_text(line2, rgb_color(200, 225, 255), 230, 250);
}

static void draw_world(const game_data &game)
{
    draw_level(game.level, game.camera);
    draw_enemies(game.level, game.camera);
    draw_snowballs(game.snowballs, game.camera);
    draw_player(game.player, game.camera);
    draw_hud(game);
}

static void draw_title()
{
    fill_rectangle(rgb_color(120, 86, 64), 0, 400, SCREEN_WIDTH, 80);
    fill_rectangle(rgb_color(245, 250, 255), 0, 400, SCREEN_WIDTH, 7);

    player_data pip = create_player(point_at(360, 370));
    pip.power = FROST;
    camera_data still = {0};
    draw_player(pip, still);
    draw_grumble(420, 370, -1);

    draw_panel("F R O S T   H O P", "Freeze the Grumbles. Use them as steps.", "Press Enter to start");
    draw_text("A/D run   W/Space jump (hold for higher)   J throw snowball", rgb_color(20, 30, 50), 170, 330);
}

void draw_game(const game_data &game)
{
    clear_screen(rgb_color(170, 210, 240));
    switch (game.screen)
    {
        case TITLE_SCREEN:
            draw_title();
            break;
        case PLAYING:
            draw_world(game);
            break;
        case LEVEL_COMPLETE:
            draw_world(game);
            if (game.level_number < LEVEL_COUNT)
                draw_panel("Level " + std::to_string(game.level_number) + " complete!",
                           "Score: " + std::to_string(game.player.score), "Press Enter for the next level");
            else
                draw_panel("You cleared every level!",
                           "Final score: " + std::to_string(game.player.score), "Press Enter to return to the title");
            break;
        case GAME_OVER:
            draw_panel("Game over", "Final score: " + std::to_string(game.player.score),
                       "Press Enter to return to the title");
            break;
    }
}
