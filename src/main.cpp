#include "edit.h"

#include <ncurses.h>

int main(int argc, char** argv)
{
    // Initialize NCURSES
    initscr();

    nonl();
    cbreak();
    curs_set(1);
    noecho();


    Editor edit(stdscr);

    edit.run();

    delwin(stdscr);
    endwin();

    return 0;
}
