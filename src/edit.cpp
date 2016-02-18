#include "edit.h"
#include "undo.h"
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
            if(y < lines.size()) {
                lines.insert(lines.begin() + y + 1, "");
            } else {
                lines.push_back("");
            }

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

        } else if (ch == KEY_BACKSPACE){
	    x--; //decrement x, then that character is deleted
	    if(x < 0){
	    	x = 0; //if x is x < 0, it crashes
		if(y > 0){
		  y--;
		  x = lines.at(y).length(); //put the cursor at the end of the next line
		}
	    }
	    lines.at(y).erase(x, 1);

	} else {
            if(x < line.length()) {
                line.insert(line.begin() + x, ch);
            } else {
                line += (char) ch;
            }

            x++;
        }

        printLines();
        wmove(window, y, x);
        wrefresh(window);
    }
}

void Editor::printLines()
{
    werase(window); //clear the screen before moving to the origin, so the junk that should be deleted while backspacing gets deleted

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

    if(x > line.length() -1 && x != 0) {
        x = line.length() - 1;
    }
}
