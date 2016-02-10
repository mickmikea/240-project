


	
	
	
	


#include <iostream>
#include "read_write_file.h"
#include <vector>
#include <ncurses.h>
// need to edit print function to specify range to print

int print_text (std::vector<std::string>, int);

int main()
{
    read_write_file rw;
    std::vector<std::string> text;
    text = rw.read_file("read");

    initscr();
    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    (void) nonl();     
    (void) cbreak();   


    curs_set(1); // mouse visible
    start_color(); // color visible

    WINDOW* window = NULL;
    window = newwin(10, 10, 0, 0);
int number_of_printed_lines =0;
    int height, width;
    int x = 0;
	int y = 0;
	int xover =0;
	int yover =0;
	move(y, x);
    height = getmaxy(stdscr);  //get the largest y value on the screen
    width = getmaxx(stdscr);   //get the largest x value on the screen
	
        //Number of rows, number of colums, starting y, starting x
        window = newwin(height, width, 0, 0);  //create the window
        int ch = ' ';

	getch();
	number_of_printed_lines  =  print_text(text, number_of_printed_lines);
	move(y, x);
    while (1)
    {
	
		refresh();

        height = getmaxy(stdscr);
        width = getmaxx(stdscr);

        ch = getch();

        if(ch == KEY_DOWN)
		{
			
			if ((getmaxy(stdscr)- y) > 3)
			{ 
				y++;
				move(y, x);
				wscrl(stdscr, 1);
				yover++;
			}
		}
		if(ch == KEY_UP)
		{
			if (y > 0)
			{ 
				y--;
				move(y, x);
				wscrl(stdscr, -1);
				yover--;
			}
			
		}
		if(ch == KEY_LEFT)
		{
			mvwprintw(stdscr, 4, 1, "Left");
			x = moveLeft(x, width);
		}
		if(ch == KEY_RIGHT)
		{
			mvwprintw(stdscr, 4, 1, "Right");
			x = moveRight(x, width);
		}
    }
	delwin(window);
	delwin(stdscr);
	endwin();


    rw.write_file(text, "out.txt");

   return 0;
}


int print_text (std::vector<std::string> text, int start)
{
int temp = text.size() + start;
	for (int i = start; i < temp; i++)
        {
            for (int j =0; j < text[i].length(); j++)
            {
			waddch(stdscr, text[i].at(j));
            }
		waddch(stdscr, '\n');
        }
return temp;
}
