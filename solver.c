#include "solver.h"
#include "field.h"

//enforces boundary conditions
void set_bounds(Field field_vx, Field field_vy, int width, int height)
{
	for(int y = 0; y < height; y++)
	{
		set(field_vx, 0, y, 0);
		set(field_vx, width, y, 0);
	}
	for(int x = 0; x < width; x++)
	{
		set(field_vy, x, 0, 0);
		set(field_vy, x, height, 0);
	}
}

//advects vx and vy
void advect(Field field_vx, Field field_vy, Field field_vx_temp, Field field_vy_temp, Field field_heat, Field field_heat_temp, int width, int height, float dt)
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width + 1; x++)
		{
			float vx = get_smooth_clipped(field_vx, x, y);
			float vy = get_smooth_clipped(field_vy, x - 0.5, y + 0.5);
			set(field_vx_temp, x, y, get_smooth_clipped(field_vx, x - vx * dt, y - vy * dt));
		}
	}
	
	for(int y = 0; y < height + 1; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float vx = get_smooth_clipped(field_vx, x + 0.5, y - 0.5);
			float vy = get_smooth_clipped(field_vy, x, y);
			set(field_vy_temp, x, y, get_smooth_clipped(field_vy, x - vx * dt, y - vy * dt));
		}
	}

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float vx = get_smooth_clipped(field_vx, x + 0.5, y);
			float vy = get_smooth_clipped(field_vy, x, y + 0.5);
			set(field_heat_temp, x, y, get_smooth_clipped(field_heat, x - vx * dt, y - vy * dt));
		}
	}
}

//solves for pressure using the gauss-seidel method
void solver_iteration(Field field_pressure, Field field_div, int width, int height)
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float sigma = get_clipped(field_pressure, x + 1, y) + get_clipped(field_pressure, x - 1, y) + get_clipped(field_pressure, x, y + 1) + get_clipped(field_pressure, x, y - 1);
			float new_pressure = (get(field_div, x, y) - sigma) * -0.25;
			set(field_pressure, x, y, new_pressure);
		}
	}
}

//calculates pressure based on the divergence of the fluid
void calculate_pressure(Field field_vx, Field field_vy, Field field_pressure, Field field_div, int width, int height)
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float div = get(field_vx, x + 1, y) - get(field_vx, x, y) + get(field_vy, x, y + 1) - get(field_vy, x, y);
			set(field_div, x, y, div);
		}
	}

	for(int i = 0; i < 8; i++)
	{
		solver_iteration(field_pressure, field_div, width, height);
	}

	//does pressure gradient solving on vx
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width - 1; x++)
		{
			float grad_x = get(field_pressure, x + 1, y) - get(field_pressure, x, y);
			set(field_vx, x + 1, y, get(field_vx, x + 1, y) - grad_x);
		}
	}
	
	//does pressure gradient solving on vy
	for(int y = 0; y < height - 1; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float grad_y = get(field_pressure, x, y + 1) - get(field_pressure, x, y);
			set(field_vy, x, y + 1, get(field_vy, x, y + 1) - grad_y);
		}
	}
}

//applies the forces in the simulation
void apply_forces(Field field_vx, Field field_vy, Field field_heat, int width, int height, float dt)
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float heat = get(field_heat, x, y);
			set(field_vy, x, y, get(field_vy, x, y) - heat * dt);
			set(field_vy, x, y + 1, get(field_vy, x, y + 1) - heat * dt);
		}
	}
}

#define DECAY_RATE 0.1
//performs chemistry
void dynamics_tick(Field field_heat, int width, int height, float dt)
{
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			float heat = get(field_heat, x, y);
			heat -= heat * dt * DECAY_RATE;
			set(field_heat, x, y, heat);
		}
	}
}

//calculates curl at point
float get_curl(int x, int y, Field field_vx, Field field_vy)
{
	return - get_clipped(field_vy, x, y) + get_clipped(field_vy, x - 1, y) + get_clipped(field_vx, x, y) - get_clipped(field_vx, x, y - 1);
}

//vorticity confinement correction
#define VC_STRENGTH 1
void vorticity_confinement(Field field_vx, Field field_vy, Field field_vx_temp, Field field_vy_temp, int width, int height, float dt)
{
	//copy data into temp
	copy(field_vx, field_vx_temp);
	copy(field_vy, field_vy_temp);

	//solve for x
	for(int y = 1; y < height; y++)
	{
		for(int x = 1; x < width; x++)
		{
			float vorticity = get_curl(x, y, field_vx, field_vy) * 4
				- get_curl(x + 1, y    , field_vx, field_vy)
				- get_curl(x - 1, y    , field_vx, field_vy)
				- get_curl(x    , y + 1, field_vx, field_vy)
				- get_curl(x    , y - 1, field_vx, field_vy);
			vorticity *= VC_STRENGTH * dt;
			set(field_vx_temp, x    , y    , get(field_vx_temp, x    , y    ) + vorticity);
			set(field_vx_temp, x    , y - 1, get(field_vx_temp, x    , y - 1) - vorticity);
			set(field_vy_temp, x    , y    , get(field_vy_temp, x    , y    ) - vorticity);
			set(field_vy_temp, x - 1, y    , get(field_vy_temp, x - 1, y    ) + vorticity);
		}
	}
}

//solves fluid dynamics for the given data fields(only field_vx and field_vy have to be initialized)
void solve(Field field_vx, Field field_vy, Field field_vx_temp, Field field_vy_temp, Field field_pressure, Field field_div, Field field_heat, Field field_heat_temp, int width, int height, float dt)
{
	advect(field_vx, field_vy, field_vx_temp, field_vy_temp, field_heat, field_heat_temp, width, height, dt);
	//swap the fields
	swap(field_vx, field_vx_temp);
	swap(field_vy, field_vy_temp);
	swap(field_heat, field_heat_temp);

	vorticity_confinement(field_vx, field_vy, field_vx_temp, field_vy_temp, width, height, dt);
	swap(field_vx, field_vx_temp);
	swap(field_vy, field_vy_temp);

	apply_forces(field_vx, field_vy, field_heat, width, height, dt);
	dynamics_tick(field_heat, width, height, dt);

	set_bounds(field_vx, field_vy, width, height);
	calculate_pressure(field_vx, field_vy, field_pressure, field_div, width, height);
	set_bounds(field_vx, field_vy, width, height);
}
