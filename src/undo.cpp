#include "undo.h"


void UndoManager::undo(WINDOW* window)
{
	if(undoActions.size() >= 1) // at least 1 thing on the stack, or else there isn't anything to undo
	{
		UndoAction currentAction = undoActions.at(undoActions.size() - 1);
		undoActions.pop_back();

		if(currentAction.mode == 0) // undo action = 0, redo action = 1
		{
			mvwdelch(window, currentAction.y, currentAction.x); // pass the window and loaction of the character

			currentAction.mode = 1; //change the mode to redo
			redoActions.push_back(currentAction); //push the character onto the redo stack, in case the undo action should be undone
		}
		else
		{
			// pass the window and loaction of the character to be redone
			mvwaddch(window, currentAction.y, currentAction.x, currentAction.ch); 

			currentAction.mode = 0;
			redoActions.push_back(currentAction);
		}
	}
}

void UndoManager::redo(WINDOW* window)
{
	if(redoActions.size() >= 1) //there has to be something on the stack for it to work
	{
		UndoAction currentAction = redoActions.at(redoActions.size() - 1);
		redoActions.pop_back();

		if(currentAction.mode == 0) // undo action = 0, redo action = 1
		{
			mvwdelch(window, currentAction.y, currentAction.x); // pass the window and loaction of the character

			currentAction.mode = 1; //change the mode to redo
			undoActions.push_back(currentAction); //push the character onto the undo stack
		}
		else
		{
			// pass the window and loaction of the character to be redone
			mvwaddch(window, currentAction.y, currentAction.x, currentAction.ch); 

			currentAction.mode = 0;
			undoActions.push_back(currentAction);
		}
	}
}

void UndoManager::addAction(UndoAction act)
{
	undoActions.push_back(act); //method to be called in ediit::run that will impliment the undo feature

	redoActions.clear();
}


