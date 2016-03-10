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

	

	wordWrap();
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

<<<<<<< HEAD
        writeKeyWordLine(i, line);//checks if it needs to highlight a keyword
   }
=======
        waddstr(window, line.c_str());

        if(i < lines.size() - 1) {
            waddch(window, '\n');
        }
    }
>>>>>>> upstream/master
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
	x = 0;
        if(y > 0) 
        {
	    if(lines.at(y).length() == 0)
	    {
	        lines.erase(lines.begin()+y);
	    }
            y--;
	    if(localY==0)
	    {
	        lineStart--;
	    }
	    else
	    {
	        localY--;
	    }
            x = lines.at(y).length() - 1; //put the cursor at the end of the next line
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
    else
    {
	y = 0;
	localY = 0;
	lineStart = 0;
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
    else
    {
	keyUp(line, keyPressed);
	x = lines.at(y).size() - 1;
    }
}

void Editor::keyRight(std::string& line, char keyPressed)
{
    if(x < line.size()) {
        x++;
    }
    else
    {
	keyDown(line, keyPressed);
	x = 0;
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
<<<<<<< HEAD
    werase(window); //erases window before prompt
=======
    y = 0;
    x = 0;
    newLine(lines.at(0), 0);
>>>>>>> upstream/master
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

    wordWrap();
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

int findLastSpace(std::string str) // finds the last white space for word wrap
{
    for(int i = str.length() - 2; i >= 0; i--)
    {
	if(str[i] == ' ' )
	{
	    return i;
	}
    }

    return -1;
}

void Editor::wordWrap() // wraps the text to the next line if it is longer than the max line length
{
    bool wrapped = false;
    for(int i = y; i < lines.size(); i++)
    {
	if(lines.at(i).length() >= COLS)
	{
	    int lastSpace = findLastSpace(lines.at(i));
	    if(lastSpace >= 0)
	    {
		wrapped = true;
		if(lastSpace < lines.at(i).length() - 1)
		{
		    std::string wrapString = lines.at(i).substr(lastSpace+1);
		    lines[i].erase(lastSpace+1);

		    if(i == lines.size() - 1)
		    {
			lines.push_back(wrapString);
		    }
		    else
		    {
			if(lines.at(i+1).empty())
			{
			    lines[i+1] = wrapString;
			}
			else
			{
			    lines[i+1].insert(0, wrapString);
			}
		    }
		    x = wrapString.length();
		}
		else
		{
		    if(i == lines.size() - 1)
		    {
			lines.push_back("");
		    }
		    else
		    {
			if(lines.at(i+1).empty())
			{
			    lines[i+1] = "";
			}
		    }
		    x = 0;
		}
	    }
	} 
    }

    if(wrapped)
    {
	int maxy = getmaxy(window);

        if(y < lines.size() - 1) 
	{
	    if(localY < maxy - 2) 
	    {
	        localY++;
	    } 
	    else 
	    {
	        lineStart++;
	    }

	    y++;
        }
    }
}
