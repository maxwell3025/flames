#ifndef FIELD_H
#define FIELD_H

typedef struct{
	float* data;
	int width;
	int height;
} Field;

//initializes a new field with a given width and height
Field new_field(int width, int height);

//copies data between 2 fields
void copy(Field a, Field b);

//swaps data in 2 fields if possible
void swap(Field a, Field b);

//sets the value of a cell in the field
void set(Field field, int x, int y, float value);

//gets a cell of a grid unchecked
float get(Field field, int x, int y);

//samples from grid while clipping x and y within bounds
float get_clipped(Field field, int x, int y);

//samples from grid with default value for out of bounds
float get_default(Field field, int x, int y, float def);

//samples from grid with constant variance and using the clipping for out of bounds
float get_smooth_clipped(Field field, float x, float y);

//samples from grid with constant variance and using a default for out of bounds
float get_smooth_default(Field field, float x, float y, float def);

#endif
