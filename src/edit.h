#ifndef EDIT_H
#define EDIT_H

#include "keybind.h"

#include <ncurses.h>
#include <string>
#include <vector>

class Editor
{
public:
    /**
     * @brief Editor::Editor Initializes a new editor for the specified ncurses WINDOW.
     *                       The Editor DOES NOT take control of the WINDOW*. It's still
     *                       the calling function's responsibility to clean up properly.
     * @param window the WINDOW to be modified by this Editor
     */
    Editor(WINDOW* window);

    /**
     * @brief Editor::run the main editor loop that handles all key events for the editor
     */
    void run();

private:
    /**
     * @brief printLines prints all of the lines contained in the 'lines' vector to the console.
     */
    void printLines();

    /**
     * @brief drawStatusBar draw the status bar on the bottom of the terminal
     */
    void drawStatusBar();

    /**
     * @brief checkLineBounds checks to be sure the cursor is within the bounds of the current line. If it's not, the cursor
     *        will be moved to be at the end of the line.
     */
    void checkLineBounds();

    /**
     * @brief setupKeybindings set up the default keybindings for the editor.
     */
    void setupKeybindings();

    /**
     * @brief backspace the callback for when backspace is pressed.
     * @param pressedKey the key pressed
     */
    void backspace(std::string& line, char pressedKey);
    void newLine(std::string& line, char pressedKey);
    void keyUp(std::string& line, char pressedKey);
    void keyDown(std::string& line, char pressedKey);
    void keyLeft(std::string& line, char pressedKey);
    void keyRight(std::string& line, char pressedKey);
    void saveFile(std::string& line, char pressedKey);
    void loadFile(std::string& file, char keyPressed);
    void exit(std::string& line, char pressedKey);

    void wordWrap();
private:
    WINDOW* window;

    /**
     * @brief running whether or not the editor is running
     */
    bool running;

    /**
     * @brief localY the y coordinate in the terminal
     */
    int localY;

    /**
     * @brief x the x character of the next character
     */
    int x;

    /**
     * @brief y the y character of the next character
     */
    int y;

    /**
     * @brief lineStart the line index to start printing from the lines vector
     */
    int lineStart;

    /**
     * @brief lines holds all of the text this editor contains.
     *
     * This is the primary storage for the contents of this editor. It's read from each iteration
     * and is used to update what's output on the screen. If it turns out that this causes really
     * bad tearing later on, it can be changed, but for now it seemed like a good idea.
     */
    std::vector<std::string> lines;

    /**
     * @brief keybindings holds all of the keybindings defined for the text editor.
     */
    std::vector<Keybind> keybindings;

    /**
     * @brief fileName the name of the file being editted.
     */
    std::string fileName;
};

#endif // EDIT_H
