#ifndef SOLVER_H
#define SOLVER_H
#include "field.h"

//solves fluid dynamics for the given data fields(only field_vx and field_vy have to be initialized)
void solve(Field field_vx, Field field_vy, Field field_vx_temp, Field field_vy_temp, Field field_pressure, Field field_div, Field field_heat, Field field_heat_temp, int width, int height, float dt);
	
#endif
