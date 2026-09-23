// program.cpp
// Frost Hop: entry point and game loop.

#include "frost_hop.h"

int main()
{
    open_window("Frost Hop", SCREEN_WIDTH, SCREEN_HEIGHT);

    level_data level = create_test_level();
    player_data player = create_player(level.spawn);

    timer frame_timer = create_timer("frame_timer");
    start_timer(frame_timer);

    while (!quit_requested())
    {
        process_events();

        double dt = timer_ticks(frame_timer) / 1000.0;
        reset_timer(frame_timer);
        if (dt > MAX_DT) dt = MAX_DT;

        update_player(player, level, dt);
        if (player.position.y > level.height * TILE_SIZE)
            place_player(player, level.spawn);   // fell in the gap: try again

        clear_screen(rgb_color(170, 210, 240));
        draw_level(level);
        draw_player(player);
        draw_text("A/D or arrows: run    W/Up/Space: jump (hold for higher)", rgb_color(20, 30, 50), 10, 10);
        draw_text("The 3-tile platform should be reachable. The 4-tile wall should not.", rgb_color(20, 30, 50), 10, 24);
        refresh_screen(60);
    }

    free_level(level);
    return 0;
}
