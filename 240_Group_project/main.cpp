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

    int number_of_printed_lines =0;
    int height, width;
    int x = 0;
    int y = 0;
    int start_print =0;
    move(y, x);
    
    height = getmaxy(stdscr);  //get the largest y value on the screen
    width = getmaxx(stdscr);   //get the largest x value on the screen
	
    int ch = ' ';
    getch();
    
    move(y, x);
    
    while (1)
    {
	start_print =  print_text(text, start_print);
	refresh();

        height = getmaxy(stdscr);
        width = getmaxx(stdscr);

        ch = getch();

        if(ch == KEY_DOWN)
	{
		if ((getmaxy(stdscr)- y) > 3)
		{ 
			y++;
			start_print++;
			move(y, x);
			wscrl(stdscr, 1);
		}
	}
	if(ch == KEY_UP)
	{
		if (y > 0)
		{ 
		y--;
		move(y, x);
		wscrl(stdscr, -1);
		}
			
	}
	if(ch == KEY_LEFT)
	{
		
	}
	if(ch == KEY_RIGHT)
	{
		
	}
	if(ch == 'q')
	{
		//exit the loop
		break;
	}
    }
	delwin(stdscr);
	endwin();


    rw.write_file(text, "out.txt");

   return 0;
}


int print_text (std::vector<std::string> text, int start)
{
int temp =  getmaxy(stdscr)+ start; 
	for (int i = start; i < temp; i++)
        {
            for (int j =0; j < text[i].length(); j++)
            {
		waddch(stdscr, text[i].at(j));
            }
		waddch(stdscr, '\n');
        }
return temp; //returns the line number it stop priting at
}
