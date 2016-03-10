#include "edit.h"
#include "read_write_file.h"
#include "undo.h"

#include <ncurses.h>
#include <functional>
#include <cstdio>

Editor::Editor(WINDOW* window)
    : window{window}, x{0}, y{0}, lineStart{0}, localY{0}, fileName{"Untitled"}, running{true}
{
    keypad(window, true);

    setupKeybindings();
}

void Editor::run()
{
    if(lines.empty()) {
        // This is the default line, because there's ALWAYS at least one line.
        lines.push_back("");
    }
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_WHITE);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_WHITE);


    while(running)
    {
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
        drawStatusBar();
        wmove(window, localY, x);
        wrefresh(window);
    }
}

void Editor::printLines()
{
    werase(window); //clear the screen before moving to the origin, so the junk that should be deleted while backspacing gets deleted

    // Move to the origin
    wmove(window, 0, 0);

    // Write the lines out
    for(int i = lineStart; i < lines.size(); i++) {
        std::string line = lines.at(i);

        writeKeyWordLine(i, line);//checks if it needs to highlight a keyword
   }
}

void Editor::drawStatusBar()
{
    int maxX = getmaxx(window);
    int maxY = getmaxy(window);

    for(int i = 0; i < maxX; i++) {
        mvwaddch(window, maxY - 1, i, ' '); // Erase the bottom line
    }

    mvwprintw(window, maxY - 1, 0, fileName.c_str());
    mvwprintw(window, maxY - 1, maxX - 10, "%d, %d", x, y);
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
    if(x < 0)
    {
        x = 0; //if x is x < 0, it crashes
        if(y > 0)
        {
	    lines.erase(lines.begin()+y);
            y--;
	    if(localY==0)
	    {
	        lineStart--;
	    }
	    else
	    {
	        localY--;
	    }
            x = lines.at(y).length(); //put the cursor at the end of the next line
        }
    }
    lines.at(y).erase(x, 1);
}

void Editor::newLine(std::string& line, char keyPressed)
{
    int maxY = getmaxy(window);

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

    if(localY < maxY - 2) {
        localY++;
    } else {
        lineStart++;
    }
}

void Editor::keyUp(std::string& line, char keyPressed)
{
    if(y > 0) {
        if(localY == 0) {
            lineStart--;
        } else {
            localY--;
        }

        y--;
    }

    checkLineBounds();
}

void Editor::keyDown(std::string& line, char keyPressed)
{
    int maxy = getmaxy(window);

    if(y < lines.size() - 1) {
        if(localY < maxy - 2) {
            localY++;
        } else {
            lineStart++;
        }

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

void Editor::saveFile(std::string& line, char keyPressed) // save file
{
    int startX = x;
    int startY = y;
    int startLocalY = localY;
    int startLineStart = lineStart;
    int startLength = lines.size();

    y = 0;
    x = 0;

    werase(window); //erase window before prompt

    newLine(lines.at(0), 0);
    lines.at(0) = "Enter a save filename: ";
    x = lines.at(0).length();
    int filenameStart = x;

    y = 0;
    localY = 0;
    lineStart = 0;

    printLines();
    drawStatusBar();
    wmove(window, localY, x);
    wrefresh(window);

    while(true)
    {
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
	    if(x >= COLS - 1)
	    {
		break;
	    }

            if(x < line.length()) {
                line.insert(line.begin() + x, ch);
            } else {
                line += (char) ch;
            }

            x++;
        }

	//if we're past the top line, go with what we've got
	if(y != 0)
	{
	    break;
	}

        printLines();
        drawStatusBar();
        wmove(window, localY, x);
        wrefresh(window);
    }

    //Get rid of any extra lines
    std::string filename = lines.at(0).substr(filenameStart);
    lines.erase(lines.begin());

    if(lines.size() > startLength)
    {
	lines.erase(lines.begin());
    }

    read_write_file::write_file(lines, filename);

    fileName = filename;

    //Restore location variables to their previous values
    x = startX;
    y = startY;
    localY = startLocalY;
    lineStart = startLineStart;

    printLines();
    drawStatusBar();
    wmove(window, localY, x);
    wrefresh(window);
}


void Editor::loadFile(std::string &fileName, char keyPressed) // load file
{
    werase(window); //erases window before prompt
    lines.at(0) = "Enter a load filename: ";
    x = lines.at(0).length();
    int filenameStart = x;

    y = 0;
    localY = 0;
    lineStart = 0;

    printLines();
    drawStatusBar();
    wmove(window, localY, x);
    wrefresh(window);

    while(running) {
        int ch = getch(); // Read the next typed character.
        std::string& line = lines.at(0); // Get the string that holds the information about the line we're on

        bool handled = false;

        for(auto keybind : keybindings) {
            if(keybind.pressedKey == ch) {
                keybind.callback(line, ch);
                handled = true;
            }
        }

        if(!handled) {
            if(x >= COLS - 1) {
               break;
            }

            if(x < line.length()) {
                line.insert(line.begin() + x, ch);
            } else {
                line += (char) ch;
            }

            x++;
        }

        //if we're past the top line, go with what we've got
        if(y != 0)
        {
            break;
        }

        printLines();
        drawStatusBar();
        wmove(window, localY, x);
        wrefresh(window);
    }

    std::string filename = lines.at(0).substr(filenameStart);
    fileName = filename;

    lines.clear();
    lines = read_write_file::read_file(filename);

    if(lines.empty()) {
        lines.push_back("");
    }

    x = 0;
    y = 0;
    localY = 0;
    lineStart = 0;

    printLines();
    drawStatusBar();
    wmove(window, localY, x);
    mvprintw(2, 2, "%s", filename.c_str());
    wrefresh(window);
}

void Editor::exit(std::string& line, char keyPressed)
{
    running = false;
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
        { KEY_F(5), std::bind(&Editor::saveFile, this, std::placeholders::_1, std::placeholders::_2) }, // save file keybind
        { KEY_F(6), std::bind(&Editor::loadFile, this, std::placeholders::_1, std::placeholders::_2) },
        { KEY_F(10), std::bind(&Editor::exit, this, std::placeholders::_1, std::placeholders::_2) },
    };

    for(auto keybind : defaultKeybindings) {
        keybindings.push_back(keybind);
    }
}

int Editor::brackets() // brackets() returns the number of matched brackets
{
    int open =0;
    int close =0;
    int total =0;

        for (int i = 0; i < lines.size(); i++){
	    std::string line = lines[i];
            for (int j =0; j < line.length(); j++){
                if (line.at(j) == '{'){
		    open++;
		}
                else if (line.at(j) == '}'){
		    close++;
		}
            }
        }
	while(1)
	{
	    if(open > 0 && close > 0){
	        total++;
	        open--;
	        close--;
            }
	    else{
		break;
            }
	}
    return total;

}

void Editor::writeKeyWordLine(int x, std::string line)
{
	int openBrac = brackets();
	int closeBrac = brackets();
	for (int i = 0; i < line.length(); i++){
	    if (line.length() - i >= 9){
                if (line.at(i) == 'c' && line.at(i+1) == 'o' && line.at(i+2) == 'n' && line.at(i+3) == 's' && line.at(i+4) == 't' && line.at(i+5) == 'e' && line.at(i+6) == 'x' && line.at(i+7) == 'p' && line.at(i+8) == 'r'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "constexpr");
                        attroff(COLOR_PAIR(1));
                        i = i +  9;
                }
                else if (line.at(i) == 'n' && line.at(i+1) == 'a' && line.at(i+2) == 'm' && line.at(i+3) == 'e' && line.at(i+4) == 's' && line.at(i+5) == 'p' && line.at(i+6) == 'a' && line.at(i+7) == 'c' && line.at(i+8) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "namespace");
                        attroff(COLOR_PAIR(1));
                        i = i +  9;
                }
                else if (line.at(i) == 'p' && line.at(i+1) == 'r' && line.at(i+2) == 'o' && line.at(i+3) == 't' && line.at(i+4) == 'e' && line.at(i+5) == 'c' && line.at(i+6) == 't' && line.at(i+7) == 'e' && line.at(i+8) == 'd'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "protected");
                        attroff(COLOR_PAIR(1));
                        i = i +  9;
                }

	    }
	    if (line.length() - i >= 10){
                if (line.at(i) == 'c' && line.at(i+1) == 'o' && line.at(i+2) == 'n' && line.at(i+3) == 's' && line.at(i+4) == 't' && line.at(i+5) == '_' && line.at(i+6) == 'c' && line.at(i+7) == 'a' && line.at(i+8) == 's' && line.at(i+9) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "const_cast");
                        attroff(COLOR_PAIR(1));
                        i = i +  10;
                }
	    }
            if (line.length() - i >= 11){
                if (line.at(i) == 's' && line.at(i+1) == 't' && line.at(i+2) == 'a' && line.at(i+3) == 't' && line.at(i+4) == 'i' && line.at(i+5) == 'c' && line.at(i+6) == '_' && line.at(i+7) == 'c' && line.at(i+8) == 'a' && line.at(i+9) == 's' && line.at(i+10) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "static_cast");
                        attroff(COLOR_PAIR(1));
                        i = i +  11;
                }
            }
            if (line.length() - i >= 12){
                if (line.at(i) == 'd' && line.at(i+1) == 'y' && line.at(i+2) == 'n' && line.at(i+3) == 'a' && line.at(i+4) == 'm' && line.at(i+5) == 'i' && line.at(i+6) == 'c' && line.at(i+7) == '_' && line.at(i+8) == 'c' && line.at(i+9) == 'a' && line.at(i+10) == 's' && line.at(i+11) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "dynamic_cast");
                        attroff(COLOR_PAIR(1));
                        i = i +  12;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'h' && line.at(i+2) == 'r' && line.at(i+3) == 'e' && line.at(i+4) == 'a' && line.at(i+5) == 'd' && line.at(i+6) == '_' && line.at(i+7) == 'l' && line.at(i+8) == 'o' && line.at(i+9) == 'c' && line.at(i+10) == 'a' && line.at(i+11) == 'l'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "thread_local");
                        attroff(COLOR_PAIR(1));
                        i = i +  12;
                }
            }
	    if (line.length() - i >= 13){
                if (line.at(i) == 's' && line.at(i+1) == 't' && line.at(i+2) == 'a' && line.at(i+3) == 't' && line.at(i+4) == 'i' && line.at(i+5) == 'c' && line.at(i+6) == '_' && line.at(i+7) == 'a' && line.at(i+8) == 's' && line.at(i+9) == 's' && line.at(i+10) == 'e' && line.at(i+11) == 'r' && line.at(i+12) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "static_assert");
                        attroff(COLOR_PAIR(1));
                        i = i +  13;
                }
	    }
        if (line.length() - i >= 16){
                if (line.at(i) == 'r' && line.at(i+1) == 'e' && line.at(i+2) == 'i' && line.at(i+3) == 'n' && line.at(i+4) == 't' && line.at(i+5) == 'e' && line.at(i+6) == 'r' && line.at(i+7) == 'p' && line.at(i+8) == 'r' && line.at(i+9) == 'e' && line.at(i+10) == 't' && line.at(i+11) == '_' && line.at(i+12) == 'c' && line.at(i+13) == 'a' && line.at(i+14) == 's' && line.at(i+15) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "reinterpret_cast");
                        attroff(COLOR_PAIR(1));
                        i = i +  16;
                }
        }

	    if (line.length() - i >= 6){
                if (line.at(i) == 's' && line.at(i+1) == 'i' && line.at(i+2) == 'g' && line.at(i+3) == 'n' && line.at(i+4) == 'e' && line.at(i+5) == 'd') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "signed");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 's' && line.at(i+1) == 'i' && line.at(i+2) == 'z' && line.at(i+3) == 'e' && line.at(i+4) == 'o' && line.at(i+5) == 'f') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "sizeof");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 's' && line.at(i+1) == 't' && line.at(i+2) == 'a' && line.at(i+3) == 't' && line.at(i+4) == 'i' && line.at(i+5) == 'c') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "static");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 's' && line.at(i+1) == 't' && line.at(i+2) == 'r' && line.at(i+3) == 'u' && line.at(i+4) == 'c' && line.at(i+5) == 't') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "struct");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'y' && line.at(i+2) == 'p' && line.at(i+3) == 'e' && line.at(i+4) == 'i' && line.at(i+5) == 'd') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "typeid");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'x' && line.at(i+1) == 'o' && line.at(i+2) == 'r' && line.at(i+3) == '_' && line.at(i+4) == 'e' && line.at(i+5) == 'q') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "xor_eq");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'd' && line.at(i+1) == 'o' && line.at(i+2) == 'u' && line.at(i+3) == 'b' && line.at(i+4) == 'l' && line.at(i+5) == 'e') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "double");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'a' && line.at(i+1) == 'n' && line.at(i+2) == 'd' && line.at(i+3) == '_' && line.at(i+4) == 'e' && line.at(i+5) == 'q') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "and_eq");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'b' && line.at(i+1) == 'i' && line.at(i+2) == 't' && line.at(i+3) == 'a' && line.at(i+4) == 'n' && line.at(i+5) == 'd') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "bitand");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'd' && line.at(i+1) == 'e' && line.at(i+2) == 'l' && line.at(i+3) == 'e' && line.at(i+4) == 't' && line.at(i+5) == 'e') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "delete");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'e' && line.at(i+1) == 'x' && line.at(i+2) == 'p' && line.at(i+3) == 'o' && line.at(i+4) == 'r' && line.at(i+5) == 't') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "export");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'e' && line.at(i+1) == 'x' && line.at(i+2) == 't' && line.at(i+3) == 'e' && line.at(i+4) == 'r' && line.at(i+5) == 'n') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "extern");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'f' && line.at(i+1) == 'r' && line.at(i+2) == 'i' && line.at(i+3) == 'e' && line.at(i+4) == 'n' && line.at(i+5) == 'd') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "friend");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'i' && line.at(i+1) == 'n' && line.at(i+2) == 'l' && line.at(i+3) == 'i' && line.at(i+4) == 'n' && line.at(i+5) == 'e') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "inline");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'n' && line.at(i+1) == 'o' && line.at(i+2) == 't' && line.at(i+3) == '_' && line.at(i+4) == 'e' && line.at(i+5) == 'Q') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "not_eq");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'p' && line.at(i+1) == 'u' && line.at(i+2) == 'b' && line.at(i+3) == 'l' && line.at(i+4) == 'i' && line.at(i+5) == 'c') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "public");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
                else if (line.at(i) == 'r' && line.at(i+1) == 'e' && line.at(i+2) == 't' && line.at(i+3) == 'u' && line.at(i+4) == 'r' && line.at(i+5) == 'n') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "return");
                        attroff(COLOR_PAIR(1));
                        i = i +  6;
                }
	    }
	    if (line.length() -i >= 7){
                if (line.at(i) == 'a' && line.at(i+1) == 'l' && line.at(i+2) == 'i' && line.at(i+3) == 'g' && line.at(i+4) == 'n' && line.at(i+5) == 'a' && line.at(i+6) == 's'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "alignas");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'a' && line.at(i+1) == 'l' && line.at(i+2) == 'i' && line.at(i+3) == 'g' && line.at(i+4) == 'n' && line.at(i+5) == 'o' && line.at(i+6) == 'f'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "alignof");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'o' && line.at(i+2) == 'n' && line.at(i+3) == 'c' && line.at(i+4) == 'e' && line.at(i+5) == 'p' && line.at(i+6) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "concept");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'd' && line.at(i+1) == 'e' && line.at(i+2) == 'f' && line.at(i+3) == 'a' && line.at(i+4) == 'u' && line.at(i+5) == 'l' && line.at(i+6) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "default");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'm' && line.at(i+1) == 'u' && line.at(i+2) == 't' && line.at(i+3) == 'a' && line.at(i+4) == 'b' && line.at(i+5) == 'l' && line.at(i+6) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "mutable");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'p' && line.at(i+1) == 'r' && line.at(i+2) == 'i' && line.at(i+3) == 'v' && line.at(i+4) == 'a' && line.at(i+5) == 't' && line.at(i+6) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "private");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'y' && line.at(i+2) == 'p' && line.at(i+3) == 'e' && line.at(i+4) == 'd' && line.at(i+5) == 'e' && line.at(i+6) == 'f'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "typedef");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'v' && line.at(i+1) == 'i' && line.at(i+2) == 'r' && line.at(i+3) == 't' && line.at(i+4) == 'u' && line.at(i+5) == 'a' && line.at(i+6) == 'l'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "virtual");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'w' && line.at(i+1) == 'c' && line.at(i+2) == 'h' && line.at(i+3) == 'a' && line.at(i+4) == 'r' && line.at(i+5) == '_' && line.at(i+6) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "wchar_t");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
                else if (line.at(i) == 'n' && line.at(i+1) == 'u' && line.at(i+2) == 'l' && line.at(i+3) == 'l' && line.at(i+4) == 'p' && line.at(i+5) == 't' && line.at(i+6) == 'r'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "nullptr");
                        attroff(COLOR_PAIR(1));
                        i = i +  7;
                }
	    }
	    if (line.length() - i >= 8)
	    {
                if (line.at(i) == 'c' && line.at(i+1) == 'h' && line.at(i+2) == 'a' && line.at(i+3) == 'r' && line.at(i+4) == '1' && line.at(i+5) == '6' && line.at(i+6) == '_' && line.at(i+7) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "char16_t");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'h' && line.at(i+2) == 'a' && line.at(i+3) == 'r' && line.at(i+4) == '3' && line.at(i+5) == '2' && line.at(i+6) == '_' && line.at(i+7) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "char32_t");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'o' && line.at(i+2) == 'n' && line.at(i+3) == 't' && line.at(i+4) == 'i' && line.at(i+5) == 'n' && line.at(i+6) == 'u' && line.at(i+7) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "continue");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'd' && line.at(i+1) == 'e' && line.at(i+2) == 'c' && line.at(i+3) == 'l' && line.at(i+4) == 't' && line.at(i+5) == 'y' && line.at(i+6) == 'p' && line.at(i+7) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "decltype");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'e' && line.at(i+1) == 'x' && line.at(i+2) == 'p' && line.at(i+3) == 'l' && line.at(i+4) == 'i' && line.at(i+5) == 'c' && line.at(i+6) == 'i' && line.at(i+7) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "explicit");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'n' && line.at(i+1) == 'o' && line.at(i+2) == 'e' && line.at(i+3) == 'x' && line.at(i+4) == 'c' && line.at(i+5) == 'e' && line.at(i+6) == 'p' && line.at(i+7) == 't'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "noexcept");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'o' && line.at(i+1) == 'p' && line.at(i+2) == 'e' && line.at(i+3) == 'r' && line.at(i+4) == 'a' && line.at(i+5) == 't' && line.at(i+6) == 'o' && line.at(i+7) == 'r'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "operator");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'r' && line.at(i+1) == 'e' && line.at(i+2) == 'g' && line.at(i+3) == 'i' && line.at(i+4) == 's' && line.at(i+5) == 't' && line.at(i+6) == 'e' && line.at(i+7) == 'r'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "register");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'r' && line.at(i+1) == 'e' && line.at(i+2) == 'q' && line.at(i+3) == 'u' && line.at(i+4) == 'i' && line.at(i+5) == 'r' && line.at(i+6) == 'e' && line.at(i+7) == 's'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "requires");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'e' && line.at(i+2) == 'm' && line.at(i+3) == 'p' && line.at(i+4) == 'l' && line.at(i+5) == 'a' && line.at(i+6) == 't' && line.at(i+7) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "template");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'y' && line.at(i+2) == 'p' && line.at(i+3) == 'e' && line.at(i+4) == 'n' && line.at(i+5) == 'a' && line.at(i+6) == 'm' && line.at(i+7) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "typename");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'v' && line.at(i+1) == 'o' && line.at(i+2) == 'l' && line.at(i+3) == 'a' && line.at(i+4) == 't' && line.at(i+5) == 'i' && line.at(i+6) == 'l' && line.at(i+7) == 'e'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "volatile");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
                else if (line.at(i) == 'u' && line.at(i+1) == 'n' && line.at(i+2) == 's' && line.at(i+3) == 'i' && line.at(i+4) == 'g' && line.at(i+5) == 'n' && line.at(i+6) == 'e' && line.at(i+7) == 'd'){
                        attron(COLOR_PAIR(1));
                        waddstr(window, "unsigned");
                        attroff(COLOR_PAIR(1));
                        i = i +  8;
                }
	    }

	    if (line.length() - i >= 2){
               if (line.at(i) == 'd' && line.at(i+1) == 'o') {
            	    attron(COLOR_PAIR(1));
        	    waddstr(window, "do");
                    attroff(COLOR_PAIR(1));
                    i = i +  2;
                }
                else if (line.at(i) == 'i' && line.at(i+1) == 'f') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "if");
                    attroff(COLOR_PAIR(1));
                    i = i +  2;
		}
                else if (line.at(i) == 'o' && line.at(i+1) == 'r') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "or");
                    attroff(COLOR_PAIR(1));
                    i = i +  2;
                }

	    }
	    if (line.length() -i >= 3){\
                if (line.at(i) == 'a' && line.at(i+1) == 'n' && line.at(i+2) == 'd') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "and");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 'a' && line.at(i+1) == 's' && line.at(i+2) == 'm') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "asm");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 'f' && line.at(i+1) == 'o' && line.at(i+2) == 'r') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "for");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 'i' && line.at(i+1) == 'n' && line.at(i+2) == 't') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "int");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 'n' && line.at(i+1) == 'e' && line.at(i+2) == 'w') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "new");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 'n' && line.at(i+1) == 'o' && line.at(i+2) == 't') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "not");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'r' && line.at(i+2) == 'y') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "try");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }
                else if (line.at(i) == 'x' && line.at(i+1) == 'o' && line.at(i+2) == 'r') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "xor");
                    attroff(COLOR_PAIR(1));
                    i = i +  3;
                }

	    }
	    if (line.length() - i >= 4){
                if (line.at(i) == 'a' && line.at(i+1) == 'u' && line.at(i+2) == 't' && line.at(i+3) == 'o') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "auto");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'b' && line.at(i+1) == 'o' && line.at(i+2) == 'o' && line.at(i+3) == 'l') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "bool");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'a' && line.at(i+2) == 's' && line.at(i+3) == 'e') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "case");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'h' && line.at(i+2) == 'a' && line.at(i+3) == 'r') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "char");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'e' && line.at(i+1) == 'l' && line.at(i+2) == 's' && line.at(i+3) == 'e') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "else");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'e' && line.at(i+1) == 'n' && line.at(i+2) == 'u' && line.at(i+3) == 'm') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "enum");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'g' && line.at(i+1) == 'o' && line.at(i+2) == 't' && line.at(i+3) == 'o') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "goto");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'l' && line.at(i+1) == 'o' && line.at(i+2) == 'n' && line.at(i+3) == 'g') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "long");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'h' && line.at(i+2) == 'i' && line.at(i+3) == 's') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "this");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'r' && line.at(i+2) == 'u' && line.at(i+3) == 'e') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "true");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }
                else if (line.at(i) == 'v' && line.at(i+1) == 'o' && line.at(i+2) == 'i' && line.at(i+3) == 'd') {
                    attron(COLOR_PAIR(1));
                    waddstr(window, "void");
                    attroff(COLOR_PAIR(1));
                    i = i +  4;
                }

	    }
    if (line.length() - i >= 5){
		if (line.at(i) == 'w' && line.at(i+1) == 'h' && line.at(i+2) == 'i' && line.at(i+3) == 'l' && line.at(i+4) == 'e') {
			attron(COLOR_PAIR(1));
			waddstr(window, "while");
			attroff(COLOR_PAIR(1));
			i = i +  5;
		}
                else if (line.at(i) == 'u' && line.at(i+1) == 'n' && line.at(i+2) == 'i' && line.at(i+3) == 'o' && line.at(i+4) == 'n') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "union");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 't' && line.at(i+1) == 'h' && line.at(i+2) == 'r' && line.at(i+3) == 'o' && line.at(i+4) == 'w') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "throw");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 's' && line.at(i+1) == 'h' && line.at(i+2) == 'o' && line.at(i+3) == 'r' && line.at(i+4) == 't') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "short");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'o' && line.at(i+1) == 'r' && line.at(i+2) == '_' && line.at(i+3) == 'e' && line.at(i+4) == 'q') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "or_eq");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'f' && line.at(i+1) == 'l' && line.at(i+2) == 'o' && line.at(i+3) == 'a' && line.at(i+4) == 't') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "float");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else  if (line.at(i) == 'f' && line.at(i+1) == 'a' && line.at(i+2) == 'l' && line.at(i+3) == 's' && line.at(i+4) == 'e') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "false");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'u' && line.at(i+1) == 's' && line.at(i+2) == 'i' && line.at(i+3) == 'n' && line.at(i+4) == 'g') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "using");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'o' && line.at(i+2) == 'n' && line.at(i+3) == 's' && line.at(i+4) == 't') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "const");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'l' && line.at(i+2) == 'a' && line.at(i+3) == 's' && line.at(i+4) == 's') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "class");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'c' && line.at(i+1) == 'a' && line.at(i+2) == 't' && line.at(i+3) == 'c' && line.at(i+4) == 'h') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "catch");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'b' && line.at(i+1) == 'r' && line.at(i+2) == 'e' && line.at(i+3) == 'a' && line.at(i+4) == 'k') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "break");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
                else if (line.at(i) == 'b' && line.at(i+1) == 'i' && line.at(i+2) == 't' && line.at(i+3) == 'o' && line.at(i+4) == 'r') {
                        attron(COLOR_PAIR(1));
                        waddstr(window, "bitor");
                        attroff(COLOR_PAIR(1));
                        i = i +  5;
                }
	    }
	    if (i < line.length())
	    {
		if (line.at(i) == '{'){
		    if(openBrac > 0){
			openBrac--;
			waddch(window, '{');
		    }
		    else{
                        attron(COLOR_PAIR(3));
                        waddch(window, '{');
                        attroff(COLOR_PAIR(3));
		    }
		}
		else if (line.at(i) == '}'){
                    if(closeBrac > 0){
                        closeBrac--;
			waddch(window, '}');
                    }
                    else{
                        attron(COLOR_PAIR(3));
                        waddch(window, '}');
                        attroff(COLOR_PAIR(3));
                    }

		}
		else{
	    	    waddch(window, line.at(i));
	   	}
	   }
	}
	if (x < lines.size() - 1) {
		waddch(window, '\n');
	}
}
