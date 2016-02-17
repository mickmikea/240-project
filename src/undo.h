#ifndef UNDO_H
#define UNDO_H
#include <vector>
#include <iostream>
#include "ncurses.h"

struct UndoAction
{
	int x, y; // coordinates of the character being undone, or replaced
	int ch;
	int mode; //either delete(undo) or place(redo)
};

class UndoManager //this class will manage the UndoAction struct
{
	public:

	void undo(WINDOW* window);
	void redo(WINDOW* window);
	void addAction(UndoAction act); // push an action, like undo or redo, onto a stack

	private:
	std::vector<UndoAction> undoActions; //vectors will act as stacks
	std::vector<UndoAction> redoActions;
};


#endif
