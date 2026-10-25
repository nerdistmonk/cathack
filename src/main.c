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
        init_pair(1, COLOR_YELLOW, COLOR_BLACK);  // Gold / Player / Merchant
        init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Food / Treats / Litter
        init_pair(3, COLOR_RED, COLOR_BLACK);     // Weapons / Stray
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK); // Boss / Panthers
        init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Water / Ranged Spitters
        init_pair(6, COLOR_WHITE, COLOR_BLACK);   // Map Walls
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
                case 'w': case 'W': move_player(0, -1); break;
                case 's': case 'S': move_player(0, 1); break;
                case 'a': case 'A': move_player(-1, 0); break;
                case 'd': case 'D': move_player(1, 0); break;

                case 'g': case 'G': pickup_item(); break;
                case 'i': case 'I': open_inventory(); break;
                case 't': case 'T': throw_item(); break;

                case '>':
                case '.':
                case 'e':
                case 'E':
                    if (player.x == stairs_x && player.y == stairs_y) {
                        player.dungeon_depth++;
                        set_log("Descended deeper into level %d!", player.dungeon_depth);
                        generate_dungeon();
                    } else {
                        set_log("You must be standing on '>' to descend.");
                    }
                    break;

                case 'q':
                    run_alive = false;
                    break;

                default:
                    break;
            }

            if (player.hp <= 0) {
                meta_stats.banked_gold += player.gold;
                save_meta();

                clear();
                mvprintw(8, 20, "=========================================");
                mvprintw(9, 20, "               YOU DIED!                 ");
                mvprintw(10, 20, "=========================================");
                mvprintw(12, 22, "Gold collected this run: %d", player.gold);
                mvprintw(13, 22, "Total Banked Gold saved: %d", meta_stats.banked_gold);
                mvprintw(15, 22, "Press any key to enter the Stat Shop...");
                refresh();
                getch();

                run_alive = false;
            }
        }
    }

    endwin();
    return 0;
}
