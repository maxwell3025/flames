#include "field.h"
#include <math.h>
#include <stdlib.h>

//initializes a new field with a given width and height
Field new_field(int width, int height)
{
	Field field = 
	{
		calloc(width * height, sizeof(float)), 
		width, 
		height
	};
	return field;
}

//swaps data in 2 fields if possible
void swap(Field a, Field b)
{
	if(a.width == b.width && a.height == b.height)
	{
		for(int i = 0; i < a.width * a.height; i++)
		{
			float temp = a.data[i];
			a.data[i] = b.data[i];
			b.data[i] = temp;
		}
	}
}

//sets the value of a cell in the field
void set(Field field, int x, int y, float value)
{
	field.data[x + field.width * y] = value;
}

//gets a cell of a grid unchecked
float get(Field field, int x, int y)
{
	return field.data[x + field.width * y];
}

//samples from grid while clipping x and y within bounds
float get_clipped(Field field, int x, int y)
{
	if(x < 0)
	{
		x = 0;
	}
	if(x >= field.width)
	{
		x = field.width - 1;
	}
	if(y < 0)
	{
		y = 0;
	}
	if(y >= field.height)
	{
		y = field.height - 1;
	}
	return field.data[x + field.width * y];
}

//samples from grid with default value for out of bounds
float get_default(Field field, int x, int y, float def)
{
	if(x < 0 || y < 0 || x >= field.width || y >= field.height)
	{
		return def;
	}
	else
	{
		return field.data[x + field.width * y];
	}
}

//samples from grid with constant variance and using the clipping for out of bounds
float get_smooth_clipped(Field field, float x, float y)
{
	int int_x = (int)round(x);
	int int_y = (int)round(y);
	float frac_x = x - int_x;
	float frac_y = y - int_y;
	float cells[] = {
		get_clipped(field, int_x-1, int_y-1),
		get_clipped(field, int_x  , int_y-1),
		get_clipped(field, int_x+1, int_y-1),
		get_clipped(field, int_x-1, int_y  ),
		get_clipped(field, int_x  , int_y  ),
		get_clipped(field, int_x+1, int_y  ),
		get_clipped(field, int_x-1, int_y+1),
		get_clipped(field, int_x  , int_y+1),
		get_clipped(field, int_x+1, int_y+1)
	};
	
	float l_x = 0.5 * (frac_x - 0.5) * (frac_x - 0.5);
	float c_x = 0.75 - frac_x * frac_x;
	float u_x = 0.5 * (frac_x + 0.5) * (frac_x + 0.5);

	float l_y = 0.5 * (frac_y - 0.5) * (frac_y - 0.5);
	float c_y = 0.75 - frac_y * frac_y;
	float u_y = 0.5 * (frac_y + 0.5) * (frac_y + 0.5);

	return
		cells[0] * l_x * l_y + 
		cells[1] * c_x * l_y + 
		cells[2] * u_x * l_y + 
		cells[3] * l_x * c_y + 
		cells[4] * c_x * c_y + 
		cells[5] * u_x * c_y + 
		cells[6] * l_x * u_y + 
		cells[7] * c_x * u_y + 
		cells[8] * u_x * u_y;
}

//samples from grid with constant variance and using a default for out of bounds
float get_smooth_default(Field field, float x, float y, float def)
{
	int int_x = (int)round(x);
	int int_y = (int)round(y);
	float frac_x = x - int_x;
	float frac_y = y - int_y;
	float cells[] = {
		get_default(field, int_x-1, int_y-1, def),
		get_default(field, int_x  , int_y-1, def),
		get_default(field, int_x+1, int_y-1, def),
		get_default(field, int_x-1, int_y  , def),
		get_default(field, int_x  , int_y  , def),
		get_default(field, int_x+1, int_y  , def),
		get_default(field, int_x-1, int_y+1, def),
		get_default(field, int_x  , int_y+1, def),
		get_default(field, int_x+1, int_y+1, def)
	};
	
	float l_x = 0.5 * (frac_x - 0.5) * (frac_x - 0.5);
	float c_x = 0.75 - frac_x * frac_x;
	float u_x = 0.5 * (frac_x + 0.5) * (frac_x + 0.5);

	float l_y = 0.5 * (frac_y - 0.5) * (frac_y - 0.5);
	float c_y = 0.75 - frac_y * frac_y;
	float u_y = 0.5 * (frac_y + 0.5) * (frac_y + 0.5);

	return
		cells[0] * l_x * l_y + 
		cells[1] * c_x * l_y + 
		cells[2] * u_x * l_y + 
		cells[3] * l_x * c_y + 
		cells[4] * c_x * c_y + 
		cells[5] * u_x * c_y + 
		cells[6] * l_x * u_y + 
		cells[7] * c_x * u_y + 
		cells[8] * u_x * u_y;
}

