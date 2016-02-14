#ifndef EDIT_H
#define EDIT_H

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
     * @brief checkLineBounds checks to be sure the cursor is within the bounds of the current line. If it's not, the cursor
     *        will be moved to be at the end of the line.
     */
    void checkLineBounds();

private:
    WINDOW* window;

    /**
     * @brief x the x character of the next character
     */
    int x;

    /**
     * @brief y the y character of the next character
     */
    int y;

    /**
     * @brief lines holds all of the text this editor contains.
     *
     * This is the primary storage for the contents of this editor. It's read from each iteration
     * and is used to update what's output on the screen. If it turns out that this causes really
     * bad tearing later on, it can be changed, but for now it seemed like a good idea.
     */
    std::vector<std::string> lines;
};

#endif // EDIT_H
