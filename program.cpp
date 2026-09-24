// program.cpp
// Frost Hop: entry point and game loop.

#include "frost_hop.h"

int main()
{
    open_window("Frost Hop", SCREEN_WIDTH, SCREEN_HEIGHT);

    game_data game;
    init_game(game);

    timer frame_timer = create_timer("frame_timer");
    start_timer(frame_timer);

    while (!quit_requested())
    {
        process_events();

        double dt = timer_ticks(frame_timer) / 1000.0;
        reset_timer(frame_timer);
        if (dt > MAX_DT) dt = MAX_DT;

        update_game(game, dt);
        draw_game(game);
        refresh_screen(60);
    }

    free_level(game.level);
    return 0;
}
