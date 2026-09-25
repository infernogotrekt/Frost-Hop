// camera.cpp
// A one-way camera: it follows Pip to the right and never scrolls back.

#include "frost_hop.h"

void update_camera(camera_data &cam, player_data &player, const level_data &level)
{
    double target = player.position.x + PLAYER_WIDTH / 2 - SCREEN_WIDTH / 2;
    if (target > cam.x)
        cam.x = target;

    double max_x = level.width * TILE_SIZE - SCREEN_WIDTH;
    if (cam.x > max_x) cam.x = max_x;
    if (cam.x < 0) cam.x = 0;

    // The left edge of the view is a wall: Pip cannot walk back off screen
    if (player.position.x < cam.x)
    {
        player.position.x = cam.x;
        if (player.velocity.x < 0)
            player.velocity.x = 0;
    }
}

// Grumbles only update near the view, so ones far ahead wait for Pip to arrive
bool in_active_range(double x, const camera_data &cam)
{
    return x + GRUMBLE_WIDTH > cam.x - ACTIVE_MARGIN && x < cam.x + SCREEN_WIDTH + ACTIVE_MARGIN;
}
