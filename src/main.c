#include "cathack.h"

int main(void) {
    srand((unsigned int)time(NULL));

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);  // Gold / Player (Caution) / Merchant
        init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Food / Treats / Litter
        init_pair(3, COLOR_RED, COLOR_BLACK);     // Weapons / Stray / Player (Critical)
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK); // Boss / Panthers
        init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Water / Ranged Spitters
        init_pair(6, COLOR_WHITE, COLOR_BLACK);   // Player (Healthy) / Walls
    }

    load_meta();

    while (true) {
        open_stat_shop();
        reset_player_for_run();

        bool run_alive = true;

        while (run_alive) {
            render_game();

            int ch = getch();
            switch (ch) {
                // Movement (WASD)
                case 'w': case 'W': move_player(0, -1); break;
                case 's': case 'S': move_player(0, 1); break;
                case 'a': case 'A': move_player(-1, 0); break;
                case 'd': case 'D': move_player(1, 0); break;

                // Actions
                case 'g': case 'G': pickup_item(); break;
                case 'i': case 'I': open_inventory(); break;
                case 't': case 'T': throw_item(); break;
                case 'p': case 'P': open_log_history(); break;

                // Descend Stairs / Victory Condition
                case '>':
                case '.':
                case 'e': case 'E':
                    if (player.x == stairs_x && player.y == stairs_y) {
                        if (player.dungeon_depth >= MAX_DEPTH) {
                            // VICTORY: Bank gold and RESET persistent stats
                            meta_stats.banked_gold += player.gold + 500;
                            meta_stats.persistent_hp = 0;
                            meta_stats.persistent_atk = 0;
                            meta_stats.persistent_def = 0;
                            save_meta();
                            record_high_score(true);

                            clear();
                            mvprintw(7, 18, "=============================================");
                            mvprintw(8, 18, "       VICTORY! YOU ESCAPED CATHACK!         ");
                            mvprintw(9, 18, "=============================================");
                            mvprintw(11, 20, "Completed all 30 Floors!");
                            mvprintw(12, 20, "Gold collected this run: %d (+500 Victory Bonus)", player.gold);
                            mvprintw(13, 20, "Total Banked Gold saved: %d", meta_stats.banked_gold);
                            mvprintw(14, 20, "Persistent stats have been reset for the next hero!");
                            mvprintw(16, 20, "Press any key to enter the Stat Shop...");
                            refresh();
                            getch();

                            run_alive = false;
                        } else {
                            player.dungeon_depth++;
                            add_log("Descended deeper into Level %d!", player.dungeon_depth);
                            generate_dungeon();
                        }
                    } else {
                        add_log("You must be standing on '>' to descend.");
                    }
                    break;

                case 'q':
                    run_alive = false;
                    break;

                default:
                    break;
            }

            // Death Check
            if (player.hp <= 0) {
                meta_stats.banked_gold += player.gold;
                save_meta();
                record_high_score(false);

                clear();
                mvprintw(8, 20, "=========================================");
                mvprintw(9, 20, "               YOU DIED!                 ");
                mvprintw(10, 20, "=========================================");
                mvprintw(12, 22, "Died on Depth: Level %d", player.dungeon_depth);
                mvprintw(13, 22, "Gold collected this run: %d", player.gold);
                mvprintw(14, 22, "Total Banked Gold saved: %d", meta_stats.banked_gold);
                mvprintw(16, 22, "Press any key to enter the Stat Shop...");
                refresh();
                getch();

                run_alive = false;
            }
        }
    }

    endwin();
    return 0;
}
