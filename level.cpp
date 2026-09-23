// level.cpp
// The tile grid: building it, asking what is where, and drawing it.

#include "frost_hop.h"
#include <cmath>

tile_kind level_data::tile_at(int col, int row) const
{
    if (col < 0 || col >= width || row < 0 || row >= height)
        return EMPTY;
    return tiles[row * width + col];
}

void level_data::set_tile(int col, int row, tile_kind kind)
{
    if (col < 0 || col >= width || row < 0 || row >= height)
        return;
    tiles[row * width + col] = kind;
}

bool level_data::is_solid(int col, int row) const
{
    if (col < 0 || col >= width)
        return true;    // the level's left and right edges act as walls
    if (row < 0 || row >= height)
        return false;   // open sky above, bottomless pits below
    return tiles[row * width + col] == GROUND;
}

bool level_data::solid_at(rectangle area) const
{
    tile_range range = tiles_under(area);
    for (int row = range.top; row <= range.bottom; row++)
        for (int col = range.left; col <= range.right; col++)
            if (is_solid(col, row))
                return true;
    return false;
}

tile_range tiles_under(const rectangle &area)
{
    tile_range range;
    range.left = (int)floor(area.x / TILE_SIZE);
    range.right = (int)floor((area.x + area.width - 0.01) / TILE_SIZE);
    range.top = (int)floor(area.y / TILE_SIZE);
    range.bottom = (int)floor((area.y + area.height - 0.01) / TILE_SIZE);
    return range;
}

// A one-screen test level with shapes that check the jump numbers:
// a 1-tile step, a 3-tile platform (reachable), a 3-tile gap and a 4-tile wall (not reachable).
level_data create_test_level()
{
    const int GROUND_ROW = 13;

    level_data level;
    level.width = SCREEN_WIDTH / TILE_SIZE;
    level.height = SCREEN_HEIGHT / TILE_SIZE;
    level.tiles = new tile_kind[level.width * level.height];
    for (int i = 0; i < level.width * level.height; i++)
        level.tiles[i] = EMPTY;

    for (int col = 0; col < level.width; col++)
        for (int row = GROUND_ROW; row < level.height; row++)
            if (col < 14 || col > 16)
                level.set_tile(col, row, GROUND);

    level.set_tile(5, GROUND_ROW - 1, GROUND);
    for (int col = 8; col <= 10; col++)
        level.set_tile(col, GROUND_ROW - 3, GROUND);
    for (int col = 21; col < level.width; col++)
        for (int row = GROUND_ROW - 4; row < GROUND_ROW; row++)
            level.set_tile(col, row, GROUND);

    level.spawn = point_at(2 * TILE_SIZE, GROUND_ROW * TILE_SIZE - PLAYER_HEIGHT);
    return level;
}

void free_level(level_data &level)
{
    delete[] level.tiles;
    level.tiles = nullptr;
    level.width = 0;
    level.height = 0;
}

static void draw_ground(const level_data &level, int col, int row, double x, double y)
{
    fill_rectangle(rgb_color(120, 86, 64), x, y, TILE_SIZE, TILE_SIZE);
    draw_rectangle(rgb_color(95, 66, 48), x, y, TILE_SIZE, TILE_SIZE);
    if (!level.is_solid(col, row - 1))
        fill_rectangle(rgb_color(245, 250, 255), x, y, TILE_SIZE, 7);   // snow on exposed tops
}

void draw_level(const level_data &level)
{
    for (int row = 0; row < level.height; row++)
        for (int col = 0; col < level.width; col++)
            if (level.tile_at(col, row) == GROUND)
                draw_ground(level, col, row, col * TILE_SIZE, row * TILE_SIZE);
}
