// physics.cpp
// The shared collision rule. Every moving thing in the game moves through move_and_collide.

#include "frost_hop.h"
#include <cmath>

bool overlaps(const rectangle &a, const rectangle &b)
{
    return a.x < b.x + b.width && a.x + a.width > b.x &&
           a.y < b.y + b.height && a.y + a.height > b.y;
}

// Moves one step of at most one pixel along one axis. Returns false if the step was blocked.
static bool try_step(double &coord, double step, point_2d &position, double width, double height,
                     const level_data &level)
{
    double old_value = coord;
    coord += step;
    if (level.solid_at(rectangle_from(position.x, position.y, width, height)))
    {
        coord = old_value;
        return false;
    }
    return true;
}

// Moves a box by its velocity, x first and then y, one pixel at a time so it never
// skips through a tile. Reports what it hit.
collision_info move_and_collide(point_2d &position, vector_2d &velocity, double width, double height,
                                const level_data &level, double dt)
{
    collision_info info = {false, false, false, -1, -1};

    double remaining = velocity.x * dt;
    while (remaining != 0)
    {
        double step = fmax(-1.0, fmin(1.0, remaining));
        if (!try_step(position.x, step, position, width, height, level))
        {
            info.hit_wall = true;
            velocity.x = 0;
            break;
        }
        remaining -= step;
    }

    remaining = velocity.y * dt;
    while (remaining != 0)
    {
        double step = fmax(-1.0, fmin(1.0, remaining));
        if (!try_step(position.y, step, position, width, height, level))
        {
            if (step > 0)
                info.on_ground = true;
            else
            {
                info.hit_ceiling = true;
                info.ceiling_col = (int)floor((position.x + width / 2) / TILE_SIZE);
                info.ceiling_row = (int)floor((position.y - 1) / TILE_SIZE);
            }
            velocity.y = 0;
            break;
        }
        remaining -= step;
    }

    return info;
}
