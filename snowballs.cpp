// snowballs.cpp
// The snowball pool: a fixed set of snowballs that get reused, so none are
// created or destroyed during play. Only MAX_SNOWBALLS can be in the air at once.

#include "frost_hop.h"

void snowball_pool::clear()
{
    for (int i = 0; i < MAX_SNOWBALLS; i++)
        items[i].active = false;
}

// Uses the first free snowball. Returns false if all are already in the air.
bool snowball_pool::throw_from(point_2d start, int facing)
{
    for (int i = 0; i < MAX_SNOWBALLS; i++)
        if (!items[i].active)
        {
            items[i].active = true;
            items[i].position = start;
            items[i].velocity = vector_to(facing * SNOWBALL_SPEED, -SNOWBALL_LIFT);
            return true;
        }
    return false;
}

static bool off_screen(const snowball &ball, const level_data &level, const camera_data &cam)
{
    return ball.position.x + SNOWBALL_SIZE < cam.x || ball.position.x > cam.x + SCREEN_WIDTH ||
           ball.position.y > level.height * TILE_SIZE;
}

void snowball_pool::update(double dt, level_data &level, const camera_data &cam, player_data &player)
{
    for (int i = 0; i < MAX_SNOWBALLS; i++)
    {
        snowball &ball = items[i];
        if (!ball.active)
            continue;

        ball.velocity.y += SNOWBALL_GRAVITY * dt;
        ball.position.x += ball.velocity.x * dt;
        ball.position.y += ball.velocity.y * dt;
        rectangle box = rectangle_from(ball.position.x, ball.position.y, SNOWBALL_SIZE, SNOWBALL_SIZE);

        for (int j = 0; j < level.enemy_count && ball.active; j++)
            if (level.enemies[j].state == WALKING && overlaps(box, enemy_box(level.enemies[j])))
            {
                freeze_enemy(level.enemies[j], player);
                ball.active = false;
            }

        // Walls, the ground and ice blocks all stop a snowball
        if (ball.active && (level.solid_at(box) || off_screen(ball, level, cam)))
            ball.active = false;
    }
}

void draw_snowballs(const snowball_pool &pool, const camera_data &cam)
{
    for (int i = 0; i < MAX_SNOWBALLS; i++)
        if (pool.items[i].active)
        {
            double x = pool.items[i].position.x - cam.x + SNOWBALL_SIZE / 2;
            double y = pool.items[i].position.y + SNOWBALL_SIZE / 2;
            fill_circle(rgb_color(250, 252, 255), x, y, SNOWBALL_SIZE / 2);
            draw_circle(rgb_color(150, 190, 230), x, y, SNOWBALL_SIZE / 2);
        }
}
