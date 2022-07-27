#include "field.h"
#include "solver.h"
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

//a filter function for dithering
#define FILTER_SIZE 128
float bayer(int x, int y)
{
	float total = 0;
	float change_size = 0.25;
	int granularity = 1;
	while(granularity < FILTER_SIZE)
	{
		bool right_half = (x & granularity) != 0;
		bool bottom_half = (y & granularity) != 0;
		if(right_half ^ bottom_half)
		{
			total += change_size * 2;
		}
		if(bottom_half)
		{
			total += change_size;
		}
		change_size *= 0.25;
		granularity *= 2;
	}

	return total;
}

//renders the field of data
#define SHADE_COUNT 10
char shade[] = " .-:*+=%#@";
int frame = 0;
int render(Field field, int width, int height, float min, float max)
{
	//render the fluid
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float value = (get(field, x, y) - min) / (max - min) * (SHADE_COUNT-1);
			int index = (int) (value + bayer(x, y));	
			if(index < 0)
			{
				index = 0;
			}
			if(index > SHADE_COUNT - 1)
			{
				index = SHADE_COUNT - 1;
			}
			mvaddch(y, x, shade[index]);
		}
	}
	mvprintw(0, 0, "|%d|", frame);
	frame++;
}

Field field_vx, field_vy, field_vx_temp, field_vy_temp, field_pressure, field_div, field_heat, field_heat_temp;
int main()
{
	//initialize curses
	initscr();
	cbreak();
	nodelay(stdscr, true);
	curs_set(0);
	int scr_width;
	int scr_height;
	getmaxyx(stdscr, scr_height, scr_width);

	//initialize fluid
	field_vx = new_field(scr_width + 1, scr_height);
	field_vy = new_field(scr_width, scr_height + 1);
	field_vx_temp = new_field(scr_width + 1, scr_height);
	field_vy_temp = new_field(scr_width, scr_height + 1);
	field_pressure = new_field(scr_width, scr_height);
	field_div = new_field(scr_width, scr_height);
	field_heat = new_field(scr_width, scr_height);
	field_heat_temp = new_field(scr_width, scr_height);

	
	while(true)
	{
		//rendering
		render(field_heat, scr_width, scr_height, 0, 1.0);
		//solving
		for(int dx = -10; dx <= 10; dx++)
		{
			set(field_heat, scr_width / 2 + dx, scr_height - 1, 1);
		}
		for(int x = 0; x < scr_width; x++)
		{
			set(field_heat, x, 0, 0);
		}
		solve(field_vx, field_vy, field_vx_temp, field_vy_temp, field_pressure, field_div, field_heat, field_heat_temp, scr_width, scr_height, 0.01);
		//screen management
		refresh();
		if(getch() != ERR)
		{
			break;
		}
		usleep(10000);
	}
	endwin();
	
	return 0;
}
