#include "edit.h"

#include <ncurses.h>

Editor::Editor(WINDOW* window)
    : window{window}, x{0}, y{0}
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
        std::string& line = lines.at(y); // Get the string that holds the information about the line we're on


        if(ch == '\r') {
            lines.insert(lines.begin() + y + 1, "");
            y++;
            x = 0;
        } else if(ch == KEY_UP) {
            if(y > 0) {
                y--;
            }

            checkLineBounds();
        } else if(ch == KEY_DOWN) {
            if(y < lines.size() - 1) {
                y++;
            }

            checkLineBounds();
        } else if(ch == KEY_LEFT) {
            if(x > 0) {
                x--;
            }
        } else if(ch == KEY_RIGHT) {
            if(x < line.size()) {
                x++;
            }
        } else {
            line.insert(line.begin() + x, ch);
            x++;
        }

        printLines();
        wmove(window, y, x);
        wrefresh(window);
    }
}

void Editor::printLines()
{
    // Move to the origin
    wmove(window, 0, 0);

    // Write the lines out
    for(int i = 0; i < lines.size(); i++) {
        std::string line = lines.at(i);

        for(auto c : line) {
            waddch(window, c);
        }

        if(i < lines.size() - 1) {
            waddch(window, '\n');
        }
    }
}

void Editor::checkLineBounds()
{
    std::string line = lines.at(y);

    if(line.empty()) {
        x = 0;
    } else {
        if(x > line.length()) {
            x = line.length();
        }
    }
}
