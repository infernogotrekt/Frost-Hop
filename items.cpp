// items.cpp
// Question blocks and pickups.

#include "frost_hop.h"

// A head bump on a question block uses it up and pays out a coin
void check_block_hits(player_data &player, level_data &level, const collision_info &info)
{
    if (!info.hit_ceiling)
        return;
    if (level.tile_at(info.ceiling_col, info.ceiling_row) != QUESTION)
        return;

    level.set_tile(info.ceiling_col, info.ceiling_row, USED_BLOCK);
    player.coins++;
    player.score += COIN_POINTS;
}

// Picks up anything Pip is overlapping
void collect_pickups(player_data &player, level_data &level)
{
    tile_range range = tiles_under(player_box(player));
    for (int row = range.top; row <= range.bottom; row++)
        for (int col = range.left; col <= range.right; col++)
            if (level.tile_at(col, row) == COIN)
            {
                level.set_tile(col, row, EMPTY);
                player.coins++;
                player.score += COIN_POINTS;
            }
}
