#include "edit.h"
#include "undo.h"
#include <ncurses.h>
#include <functional>
#include <cstdio>

Editor::Editor(WINDOW* window)
    : window{window}, x{0}, y{0}
{
    scrollok(stdscr, true);
    keypad(stdscr, true);

    // This is the default line, because there's ALWAYS at least one line.
    lines.push_back("");

    setupKeybindings();
}

void Editor::run()
{
    while(true)
    {
        int maxY = getmaxy(window);
        int maxX = getmaxx(window);

        int ch = getch(); // Read the next typed character.
        std::string& line = lines.at(y); // Get the string that holds the information about the line we're on

        bool handled = false;

        for(auto keybind : keybindings) {
            if(keybind.pressedKey == ch) {
                keybind.callback(line, ch);
                handled = true;
            }
        }

        if(!handled) {
            if(x < line.length()) {
                line.insert(line.begin() + x, ch);
            } else {
                line += (char) ch;
            }

            x++;
        }

        printLines();
        wmove(window, y > maxY ? maxY - 1 : y, x);
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

    if(line.empty()) {
        x = 0;
    } else {
        if(x > line.length()) {
            x = line.length();
        }
    }
}

void Editor::backspace(std::string& line, char keyPressed)
{
    x--; //decrement x, then that character is deleted
    if(x < 0) {
        x = 0; //if x is x < 0, it crashes
        if(y > 0) {
            y--;
            x = lines.at(y).length(); //put the cursor at the end of the next line
        }
    }
    lines.at(y).erase(x, 1);
}

void Editor::newLine(std::string& line, char keyPressed)
{
    std::string insertion = "";

    if(!line.empty()) {
        if(x < line.length()) {
            insertion = line.substr(x);
            line = line.substr(0, x);
        }
    }

    lines.insert(lines.begin() + y + 1, insertion);
    y++;
    x = 0;
}

void Editor::keyUp(std::string& line, char keyPressed)
{
    if(y > 0) {
        y--;
    }

    checkLineBounds();
}

void Editor::keyDown(std::string& line, char keyPressed)
{
    if(y < lines.size() - 1) {
        y++;
    }

    checkLineBounds();
}

void Editor::keyLeft(std::string& line, char keyPressed)
{
    if(x > 0) {
        x--;
    }
}

void Editor::keyRight(std::string& line, char keyPressed)
{
    if(x < line.size()) {
        x++;
    }
}

void Editor::setupKeybindings()
{
    std::vector<Keybind> defaultKeybindings {
        { KEY_BACKSPACE, std::bind(&Editor::backspace, this, std::placeholders::_1, std::placeholders::_2) }, // Backspace keybind
        { '\r', std::bind(&Editor::newLine, this, std::placeholders::_1, std::placeholders::_2) },
        { KEY_UP, std::bind(&Editor::keyUp, this, std::placeholders::_1, std::placeholders::_2) },
        { KEY_DOWN, std::bind(&Editor::keyDown, this, std::placeholders::_1, std::placeholders::_2) },
        { KEY_LEFT, std::bind(&Editor::keyLeft, this, std::placeholders::_1, std::placeholders::_2) },
        { KEY_RIGHT, std::bind(&Editor::keyRight, this, std::placeholders::_1, std::placeholders::_2) },
    };

    for(auto keybind : defaultKeybindings) {
        keybindings.push_back(keybind);
    }
}
