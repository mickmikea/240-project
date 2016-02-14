#include "edit.h"

#include <ncurses.h>

int main()
{
    initscr();
    start_color();
    nonl();
    cbreak();
    curs_set(1);

    Editor edit(stdscr);
    edit.run();

    delwin(stdscr);
    endwin();

    return 0;
}
