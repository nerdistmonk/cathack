#include "cathack.h"

void show_title_screen(void) {
    clear();

    // ASCII Art Cat Logo
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(3, 27, "/\\_/\\");
    mvprintw(4, 26, "( ^.^ )");
    mvprintw(5, 26, " >   < ");
    attroff(COLOR_PAIR(1) | A_BOLD);

    // Title & Subtitle
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(8, 20, "=========================================");
    mvprintw(9, 20, "               C A T H A C K             ");
    mvprintw(10, 20, "=========================================");
    attroff(COLOR_PAIR(6) | A_BOLD);

    attron(COLOR_PAIR(5));
    mvprintw(12, 12, "Version 2.0 by R. Shaklfrd and Gemini Flavorous Maximus.");
    attroff(COLOR_PAIR(5));

    // Call to Action
    attron(A_BLINK | A_BOLD);
    mvprintw(16, 25, "Press any key to enter the alley...");
    attroff(A_BLINK | A_BOLD);

    mvprintw(20, 22, "[WASD]: Move | [i]: Inv | [t]: Throw");

    refresh();
    getch();
}
