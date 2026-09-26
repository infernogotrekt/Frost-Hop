// player.cpp
// Pip: reading input, running, jumping, landing and drawing.

#include "frost_hop.h"
#include <cmath>

// Moves value toward target by at most amount, without overshooting
static double approach(double value, double target, double amount)
{
    if (value < target)
        return fmin(value + amount, target);
    return fmax(value - amount, target);
}

// Puts Pip at a spawn point, standing still. Power, lives, score and coins are kept.
void place_player(player_data &player, point_2d spawn)
{
    player.position = spawn;
    player.velocity = vector_to(0, 0);
    player.state = IDLE;
    player.on_ground = false;
    player.facing = 1;
    player.jump_held = false;
    player.coyote_timer = 0;
    player.invincible_timer = 0;
}

// A fresh Pip for a new game
player_data create_player(point_2d spawn)
{
    player_data player;
    player.power = SMALL;
    player.lives = START_LIVES;
    player.coins = 0;
    player.score = 0;
    place_player(player, spawn);
    return player;
}

rectangle player_box(const player_data &player)
{
    return rectangle_from(player.position.x, player.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
}

void handle_input(player_data &player, snowball_pool &snowballs, double dt)
{
    int direction = 0;
    if (key_down(A_KEY) || key_down(LEFT_KEY)) direction--;
    if (key_down(D_KEY) || key_down(RIGHT_KEY)) direction++;

    if (direction != 0)
    {
        player.facing = direction;
        player.velocity.x = approach(player.velocity.x, direction * RUN_SPEED, RUN_ACCEL * dt);
    }
    else
        player.velocity.x = approach(player.velocity.x, 0, RUN_DECEL * dt);

    bool jump_pressed = key_typed(W_KEY) || key_typed(UP_KEY) || key_typed(SPACE_KEY);
    bool jump_down = key_down(W_KEY) || key_down(UP_KEY) || key_down(SPACE_KEY);

    // Coyote time: a jump still counts for a moment after leaving a ledge
    if (jump_pressed && player.coyote_timer > 0)
    {
        player.velocity.y = -JUMP_SPEED;
        player.coyote_timer = 0;
        player.jump_held = true;
    }

    // Variable jump height: letting go early cuts the rise short
    if (player.jump_held && !jump_down)
    {
        player.jump_held = false;
        if (player.velocity.y < 0)
            player.velocity.y *= JUMP_CUT;
    }

    // With the Frost Flower, J throws a snowball from Pip's front
    if (key_typed(J_KEY) && player.power == FROST)
    {
        double x = player.facing > 0 ? player.position.x + PLAYER_WIDTH : player.position.x - SNOWBALL_SIZE;
        snowballs.throw_from(point_at(x, player.position.y + 8), player.facing);
    }
}

void apply_physics(player_data &player, double dt)
{
    player.velocity.y += GRAVITY * dt;
    if (player.velocity.y > MAX_FALL_SPEED)
        player.velocity.y = MAX_FALL_SPEED;
}

static void update_state(player_data &player)
{
    if (!player.on_ground)
        player.state = player.velocity.y < 0 ? JUMPING : FALLING;
    else if (fabs(player.velocity.x) > 10)
        player.state = RUNNING;
    else
        player.state = IDLE;
}

collision_info update_player(player_data &player, const level_data &level, snowball_pool &snowballs, double dt)
{
    collision_info info = {false, false, false, -1, -1};
    if (player.state == DEAD)
        return info;

    handle_input(player, snowballs, dt);
    apply_physics(player, dt);

    info = move_and_collide(player.position, player.velocity,
                                           PLAYER_WIDTH, PLAYER_HEIGHT, level, dt);
    player.on_ground = info.on_ground;
    if (player.on_ground)
        player.coyote_timer = COYOTE_TIME;
    else
        player.coyote_timer -= dt;
    if (player.invincible_timer > 0)
        player.invincible_timer -= dt;

    update_state(player);
    return info;
}

// A hit with the Frost Flower costs the power. A hit without it costs a life.
void damage_player(player_data &player)
{
    if (player.power == FROST)
    {
        player.power = SMALL;
        player.invincible_timer = INVINCIBLE_TIME;
    }
    else
        kill_player(player);
}

void kill_player(player_data &player)
{
    player.state = DEAD;
    player.velocity = vector_to(0, 0);
}

void draw_player(const player_data &player, const camera_data &cam)
{
    if (player.invincible_timer > 0 && fmod(player.invincible_timer, 0.2) < 0.1)
        return;   // flashing after a hit

    double x = player.position.x - cam.x;
    double y = player.position.y;

    color body = player.state == DEAD ? rgb_color(150, 150, 150) : rgb_color(235, 130, 60);
    fill_rectangle(body, x, y, PLAYER_WIDTH, PLAYER_HEIGHT);
    fill_rectangle(rgb_color(90, 60, 40), x, y + PLAYER_HEIGHT - 5, PLAYER_WIDTH, 5);   // boots

    double eye_x = player.facing > 0 ? x + 14 : x + 4;
    double pupil_x = player.facing > 0 ? eye_x + 3 : eye_x;
    fill_rectangle(rgb_color(255, 255, 255), eye_x, y + 6, 6, 7);
    fill_rectangle(rgb_color(20, 20, 20), pupil_x, y + 8, 3, 4);

    if (player.power == FROST)   // blue scarf, tail blowing behind
    {
        color scarf = rgb_color(60, 130, 230);
        fill_rectangle(scarf, x - 1, y + 15, PLAYER_WIDTH + 2, 5);
        fill_rectangle(scarf, player.facing > 0 ? x - 7 : x + PLAYER_WIDTH + 1, y + 15, 6, 9);
    }
}
