// enemies.cpp
// Grumbles: walking, turning at walls, getting stomped and drawing.

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

static void squash(enemy &grumble, player_data &player)
{
    grumble.state = SQUASHED;
    grumble.timer = SQUASH_TIME;
    player.velocity.y = -STOMP_BOUNCE;
    player.score += STOMP_POINTS;
}

static void update_walking(enemy &grumble, const level_data &level, double dt)
{
    grumble.velocity.x = grumble.direction * GRUMBLE_SPEED;
    grumble.velocity.y += GRAVITY * dt;
    if (grumble.velocity.y > MAX_FALL_SPEED)
        grumble.velocity.y = MAX_FALL_SPEED;

    collision_info info = move_and_collide(grumble.position, grumble.velocity,
                                           GRUMBLE_WIDTH, GRUMBLE_HEIGHT, level, dt);
    if (info.hit_wall)
        grumble.direction = -grumble.direction;
}

void update_enemies(level_data &level, const camera_data &cam, double dt)
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
        else
            update_walking(grumble, level, dt);

        if (grumble.position.y > level.height * TILE_SIZE)
            grumble.state = GONE;   // fell into a pit
    }
}

// old_bottom is where Pip's feet were before this frame's move, so a fast fall
// onto a Grumble still counts as a stomp
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
        else
        {
            damage_player(player);
            return;
        }
    }
}

static void draw_grumble(double x, double y, int direction)
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
        else
            draw_grumble(x, y, grumble.direction);
    }
}
