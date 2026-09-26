// items.cpp
// Question blocks and pickups.

#include "frost_hop.h"

// A head bump on a question block uses it up. It releases a Frost Flower on top
// if the level file marked it 'f', otherwise it pays out a coin.
void check_block_hits(player_data &player, level_data &level, const collision_info &info)
{
    int col = info.ceiling_col;
    int row = info.ceiling_row;
    if (!info.hit_ceiling || level.tile_at(col, row) != QUESTION)
        return;

    level.set_tile(col, row, USED_BLOCK);
    if (level.has_flower[row * level.width + col])
    {
        if (level.tile_at(col, row - 1) == EMPTY)
            level.set_tile(col, row - 1, FLOWER);
    }
    else
    {
        player.coins++;
        player.score += COIN_POINTS;
    }
}

// Picks up anything Pip is overlapping
void collect_pickups(player_data &player, level_data &level)
{
    tile_range range = tiles_under(player_box(player));
    for (int row = range.top; row <= range.bottom; row++)
        for (int col = range.left; col <= range.right; col++)
        {
            tile_kind kind = level.tile_at(col, row);
            if (kind == COIN)
            {
                level.set_tile(col, row, EMPTY);
                player.coins++;
                player.score += COIN_POINTS;
            }
            else if (kind == FLOWER)
            {
                level.set_tile(col, row, EMPTY);
                player.power = FROST;
                player.score += FLOWER_POINTS;
            }
        }
}
