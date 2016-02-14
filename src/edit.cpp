#include "edit.h"

#include <ncurses.h>

Editor::Editor(WINDOW* window)
    : window{window}
{
    scrollok(stdscr, true);
    keypad(stdscr, true);

    // This is the default line, because there's ALWAYS at least one line.
    lines.push_back("");
}

void Editor::run()
{
    while(true)
    {
        int ch = getch(); // Read the next typed character.

    }
}
