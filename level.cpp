// level.cpp
// The tile grid: loading it from a file, asking what is where, and drawing it.

#include "frost_hop.h"
#include <cmath>
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;
using std::getline;
using std::ifstream;

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
    tile_kind kind = tiles[row * width + col];
    return kind == GROUND || kind == QUESTION || kind == USED_BLOCK || kind == SPIKE;
}

// The shared collision rule: solid tiles, plus every frozen Grumble.
// Pip, walking Grumbles, falling ice blocks and snowballs all ask this one question.
bool level_data::solid_at(rectangle area, int ignore_enemy) const
{
    tile_range range = tiles_under(area);
    for (int row = range.top; row <= range.bottom; row++)
        for (int col = range.left; col <= range.right; col++)
            if (is_solid(col, row))
                return true;

    for (int i = 0; i < enemy_count; i++)
        if (i != ignore_enemy && enemies[i].state == FROZEN && overlaps(area, enemy_box(enemies[i])))
            return true;
    return false;
}

bool level_data::touching(rectangle area, tile_kind kind) const
{
    tile_range range = tiles_under(area);
    for (int row = range.top; row <= range.bottom; row++)
        for (int col = range.left; col <= range.right; col++)
            if (tile_at(col, row) == kind)
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

level_data empty_level()
{
    level_data level;
    level.tiles = nullptr;
    level.has_flower = nullptr;
    level.enemies = nullptr;
    level.enemy_count = 0;
    level.width = 0;
    level.height = 0;
    level.spawn = point_at(TILE_SIZE, TILE_SIZE);
    return level;
}

// A plain floor, so the game still runs if a level file is missing
static level_data fallback_level()
{
    level_data level = empty_level();
    level.width = SCREEN_WIDTH / TILE_SIZE;
    level.height = SCREEN_HEIGHT / TILE_SIZE;
    level.tiles = new tile_kind[level.width * level.height];
    level.has_flower = new bool[level.width * level.height];
    level.enemies = new enemy[0];
    for (int i = 0; i < level.width * level.height; i++)
    {
        level.tiles[i] = i / level.width >= level.height - 2 ? GROUND : EMPTY;
        level.has_flower[i] = false;
    }
    level.spawn = point_at(2 * TILE_SIZE, (level.height - 2) * TILE_SIZE - PLAYER_HEIGHT);
    return level;
}

// Turns one character of the level file into a tile, or places the player
static void read_cell(level_data &level, char c, int col, int row)
{
    int i = row * level.width + col;
    double x = col * TILE_SIZE;
    double y = row * TILE_SIZE;

    level.tiles[i] = EMPTY;
    level.has_flower[i] = false;
    switch (c)
    {
        case '#': level.tiles[i] = GROUND; break;
        case '?': level.tiles[i] = QUESTION; break;
        case 'f': level.tiles[i] = QUESTION; level.has_flower[i] = true; break;
        case 'o': level.tiles[i] = COIN; break;
        case 'F': level.tiles[i] = FLAG; break;
        case '^': level.tiles[i] = SPIKE; break;
        case 'g':
            level.enemies[level.enemy_count] =
                create_grumble(point_at(x + (TILE_SIZE - GRUMBLE_WIDTH) / 2, y + TILE_SIZE - GRUMBLE_HEIGHT));
            level.enemy_count++;
            break;
        case 'P':
            level.spawn = point_at(x + (TILE_SIZE - PLAYER_WIDTH) / 2, y + TILE_SIZE - PLAYER_HEIGHT);
            break;
        case '.': case ' ': case '\r': break;
        default:
            cout << "Unknown tile '" << c << "' at column " << col << ", row " << row
                 << " (treated as empty)" << endl;
    }
}

// Reads the file twice: once to measure it and count Grumbles so the arrays can be sized
// exactly, once to fill them
level_data load_level(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Could not open " << filename << ", using a plain floor instead" << endl;
        return fallback_level();
    }

    level_data level = empty_level();
    int grumbles = 0;
    string line;
    while (getline(file, line))
    {
        if ((int)line.length() > level.width)
            level.width = line.length();
        for (char c : line)
            if (c == 'g') grumbles++;
        level.height++;
    }

    level.tiles = new tile_kind[level.width * level.height];
    level.has_flower = new bool[level.width * level.height];
    level.enemies = new enemy[grumbles];   // enemy_count climbs back to this as they are read

    file.clear();
    file.seekg(0);
    int row = 0;
    while (row < level.height && getline(file, line))
    {
        for (int col = 0; col < level.width; col++)
            read_cell(level, col < (int)line.length() ? line[col] : '.', col, row);
        row++;
    }
    return level;
}

void free_level(level_data &level)
{
    delete[] level.tiles;
    delete[] level.has_flower;
    delete[] level.enemies;
    level.tiles = nullptr;
    level.has_flower = nullptr;
    level.enemies = nullptr;
    level.enemy_count = 0;
    level.enemies = nullptr;
    level.enemy_count = 0;
    level.width = 0;
    level.height = 0;
}

static void draw_block(color fill, color edge, double x, double y)
{
    fill_rectangle(fill, x, y, TILE_SIZE, TILE_SIZE);
    draw_rectangle(edge, x, y, TILE_SIZE, TILE_SIZE);
}

static void draw_tile(const level_data &level, int col, int row, double x, double y)
{
    switch (level.tile_at(col, row))
    {
        case GROUND:
            draw_block(rgb_color(120, 86, 64), rgb_color(95, 66, 48), x, y);
            if (!level.is_solid(col, row - 1))
                fill_rectangle(rgb_color(245, 250, 255), x, y, TILE_SIZE, 7);   // snow on exposed tops
            break;
        case QUESTION:
            draw_block(rgb_color(235, 180, 40), rgb_color(150, 100, 20), x, y);
            draw_text("?", rgb_color(90, 50, 10), x + 13, y + 12);
            break;
        case USED_BLOCK:
            draw_block(rgb_color(150, 110, 70), rgb_color(100, 70, 40), x, y);
            break;
        case COIN:
            fill_circle(rgb_color(250, 205, 40), x + 16, y + 16, 8);
            fill_circle(rgb_color(255, 240, 150), x + 13, y + 13, 3);
            break;
        case SPIKE:
            for (int i = 0; i < 2; i++)
                fill_triangle(rgb_color(150, 155, 170), x + i * 16, y + TILE_SIZE,
                              x + i * 16 + 8, y + 6, x + i * 16 + 16, y + TILE_SIZE);
            break;
        case FLAG:
            fill_rectangle(rgb_color(90, 90, 90), x + 14, y, 4, TILE_SIZE);
            if (level.tile_at(col, row - 1) != FLAG)   // top of the pole
            {
                fill_triangle(rgb_color(40, 170, 90), x + 18, y + 2, x + 18, y + 22, x + 42, y + 12);
                fill_circle(rgb_color(250, 205, 40), x + 16, y, 4);
            }
            break;
        case FLOWER:
            fill_rectangle(rgb_color(60, 150, 70), x + 15, y + 16, 3, 16);        // stem
            for (int i = 0; i < 4; i++)                                            // petals
                fill_circle(rgb_color(130, 200, 255), x + 16 + (i % 2 == 0 ? (i - 1) * 7 : 0),
                            y + 12 + (i % 2 == 1 ? (i - 2) * 7 : 0), 5);
            fill_circle(rgb_color(255, 255, 255), x + 16, y + 12, 4);
            break;
        default:
            break;
    }
}

// Draws only the columns the camera can see
void draw_level(const level_data &level, const camera_data &cam)
{
    int first_col = (int)floor(cam.x / TILE_SIZE);
    int last_col = first_col + SCREEN_WIDTH / TILE_SIZE + 1;
    for (int row = 0; row < level.height; row++)
        for (int col = first_col; col <= last_col; col++)
            draw_tile(level, col, row, col * TILE_SIZE - cam.x, row * TILE_SIZE);
}
