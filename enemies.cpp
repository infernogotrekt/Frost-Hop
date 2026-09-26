// enemies.cpp
// Grumbles: walking, turning at walls, freezing, thawing, getting stomped and drawing.

#include "frost_hop.h"
#include <cmath>

enemy create_grumble(point_2d position)
{
    enemy grumble;
    grumble.position = position;
    grumble.velocity = vector_to(0, 0);
    grumble.state = WALKING;
    grumble.direction = -1;     // Grumbles start walking toward Pip
    grumble.timer = 0;
    return grumble;
}

rectangle enemy_box(const enemy &grumble)
{
    return rectangle_from(grumble.position.x, grumble.position.y, GRUMBLE_WIDTH, GRUMBLE_HEIGHT);
}

// A snowball turns a walking Grumble into a solid ice block. It keeps its direction
// so it walks the same way when it thaws.
void freeze_enemy(enemy &grumble, player_data &player)
{
    grumble.state = FROZEN;
    grumble.timer = FREEZE_TIME;
    grumble.velocity.x = 0;
    player.score += FREEZE_POINTS;
}

static void squash(enemy &grumble, player_data &player)
{
    grumble.state = SQUASHED;
    grumble.timer = SQUASH_TIME;
    player.velocity.y = -STOMP_BOUNCE;
    player.score += STOMP_POINTS;
}

// True when Pip's feet rest on top of this Grumble
static bool standing_on(const player_data &player, const enemy &grumble)
{
    rectangle pip = player_box(player);
    rectangle ice = enemy_box(grumble);
    bool across = pip.x < ice.x + ice.width && pip.x + pip.width > ice.x;
    double gap = ice.y - (pip.y + pip.height);
    return player.on_ground && across && gap >= -1 && gap <= 2;
}

// Walking and frozen Grumbles both fall. Frozen ones do not move sideways.
static collision_info fall_and_move(level_data &level, int index, double dt)
{
    enemy &grumble = level.enemies[index];
    grumble.velocity.y += GRAVITY * dt;
    if (grumble.velocity.y > MAX_FALL_SPEED)
        grumble.velocity.y = MAX_FALL_SPEED;
    return move_and_collide(grumble.position, grumble.velocity, GRUMBLE_WIDTH, GRUMBLE_HEIGHT, level, dt, index);
}

static void update_walking(level_data &level, int index, double dt)
{
    enemy &grumble = level.enemies[index];
    grumble.velocity.x = grumble.direction * GRUMBLE_SPEED;
    collision_info info = fall_and_move(level, index, dt);
    if (info.hit_wall)
        grumble.direction = -grumble.direction;   // walls and frozen Grumbles turn it around
}

static void update_frozen(level_data &level, int index, player_data &player, double dt)
{
    enemy &grumble = level.enemies[index];
    fall_and_move(level, index, dt);

    grumble.timer -= dt;
    if (grumble.timer > 0)
        return;

    if (standing_on(player, grumble))
        squash(grumble, player);   // thawing under Pip counts as a stomp, never a hit
    else
        grumble.state = WALKING;
}

void update_enemies(level_data &level, player_data &player, const camera_data &cam, double dt)
{
    for (int i = 0; i < level.enemy_count; i++)
    {
        enemy &grumble = level.enemies[i];
        if (grumble.state == GONE || !in_active_range(grumble.position.x, cam))
            continue;

        if (grumble.state == SQUASHED)
        {
            grumble.timer -= dt;
            if (grumble.timer <= 0)
                grumble.state = GONE;
        }
        else if (grumble.state == FROZEN)
            update_frozen(level, i, player, dt);
        else
            update_walking(level, i, dt);

        if (grumble.position.y > level.height * TILE_SIZE)
            grumble.state = GONE;   // fell into a pit
    }
}

// old_bottom is where Pip's feet were before this frame's move, so a fast fall
// onto a Grumble still counts as a stomp. Frozen Grumbles are solid, so they never touch Pip here.
void check_enemy_contact(player_data &player, level_data &level, double old_bottom)
{
    if (player.state == DEAD)
        return;

    rectangle pip = player_box(player);
    for (int i = 0; i < level.enemy_count; i++)
    {
        enemy &grumble = level.enemies[i];
        if (grumble.state != WALKING || !overlaps(pip, enemy_box(grumble)))
            continue;

        if (player.velocity.y > 0 && old_bottom <= grumble.position.y + 4)
            squash(grumble, player);
        else if (player.invincible_timer <= 0)
        {
            damage_player(player);
            return;
        }
    }
}

void draw_grumble(double x, double y, int direction)
{
    color snow = rgb_color(240, 245, 255);
    color dark = rgb_color(30, 30, 40);
    fill_circle(snow, x + 14, y + 20, 10);                  // body
    fill_circle(snow, x + 14, y + 8, 8);                    // head
    double face = x + 14 + direction * 2;
    fill_circle(dark, face - 3, y + 8, 1.5);                // eyes
    fill_circle(dark, face + 3, y + 8, 1.5);
    draw_line(dark, face - 6, y + 3, face - 1, y + 6);      // angry brows
    draw_line(dark, face + 6, y + 3, face + 1, y + 6);
    fill_circle(dark, x + 14, y + 17, 1.5);                 // buttons
    fill_circle(dark, x + 14, y + 23, 1.5);
}

static void draw_ice(const enemy &grumble, double x, double y)
{
    draw_grumble(x, y, grumble.direction);
    bool flashing = grumble.timer < THAW_WARNING && fmod(grumble.timer, 0.2) < 0.1;
    fill_rectangle(rgba_color(150, 210, 255, flashing ? 70 : 170), x - 1, y - 1, GRUMBLE_WIDTH + 2, GRUMBLE_HEIGHT + 1);
    draw_rectangle(rgb_color(70, 140, 210), x - 1, y - 1, GRUMBLE_WIDTH + 2, GRUMBLE_HEIGHT + 1);
}

void draw_enemies(const level_data &level, const camera_data &cam)
{
    for (int i = 0; i < level.enemy_count; i++)
    {
        const enemy &grumble = level.enemies[i];
        double x = grumble.position.x - cam.x;
        double y = grumble.position.y;
        if (grumble.state == GONE || x < -TILE_SIZE || x > SCREEN_WIDTH)
            continue;

        if (grumble.state == SQUASHED)
            fill_rectangle(rgb_color(240, 245, 255), x, y + GRUMBLE_HEIGHT - 8, GRUMBLE_WIDTH, 8);
        else if (grumble.state == FROZEN)
            draw_ice(grumble, x, y);
        else
            draw_grumble(x, y, grumble.direction);
    }
}
