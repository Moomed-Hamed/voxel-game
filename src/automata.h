#include "items.h"

#define MAX_CELL_ARENAS 16

#define CELL_ARENA_X 9
#define CELL_ARENA_Z 9
#define CELL_ARENA_Y 9
#define NUM_ARENA_CELLS (CELL_ARENA_X * CELL_ARENA_Z * CELL_ARENA_Y)
#define CELL_INDEX(x,y,z) (((x) + (CELL_ARENA_X * (z))) + ((CELL_ARENA_X * CELL_ARENA_Z) * (y)))

typedef uint8 MatID;

#define MAT_SAND  1
#define MAT_WATER 2
#define MAT_FIRE  3

struct Cell
{
	MatID type;
	bool is_updated;
};

struct Cell_Arena
{
	uvec3 coords;
	Cell cells[NUM_ARENA_CELLS];
};

void init(Cell_Arena* arena, uvec3 coords)
{
	*arena = { coords };
}

void update(Cell_Arena* arena, float dtime)
{
	static float timer = 0;
	if (timer < .1)
	{
		timer += dtime;
		return;
	} timer = 0;

	Cell* cells = arena->cells;

	// boundaries are not processed, not sure if this is a good idea
	for (uint x = 0; x < CELL_ARENA_X; ++x) {
	for (uint z = 0; z < CELL_ARENA_Z; ++z) {
	for (uint y = CELL_ARENA_Y - 1; y >0; --y)
	{
		uint index = CELL_INDEX(x, y, z);
		
		if (cells[CELL_INDEX(x, y, z)].is_updated) continue;
		if (x == 0 || z == 0 || y == 0 || x == CELL_ARENA_X - 1 || z == CELL_ARENA_Z - 1 || y == CELL_ARENA_Y - 1) continue;

		switch (cells[CELL_INDEX(x,y,z)].type)
		{
			case MAT_SAND:
			{
				cells[index].type = NONE;

				if      (cells[CELL_INDEX(x + 0, y - 1, z + 0)].type == NONE) index = CELL_INDEX(x + 0, y - 1, z + 0);
				else if (cells[CELL_INDEX(x + 0, y - 1, z + 1)].type == NONE) index = CELL_INDEX(x + 0, y - 1, z + 1);
				else if (cells[CELL_INDEX(x + 0, y - 1, z - 1)].type == NONE) index = CELL_INDEX(x + 0, y - 1, z - 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z + 0)].type == NONE) index = CELL_INDEX(x + 1, y - 1, z + 0);
				else if (cells[CELL_INDEX(x - 1, y - 1, z + 0)].type == NONE) index = CELL_INDEX(x - 1, y - 1, z + 0);
				else if (cells[CELL_INDEX(x - 1, y - 1, z - 1)].type == NONE) index = CELL_INDEX(x - 1, y - 1, z - 1);
				else if (cells[CELL_INDEX(x - 1, y - 1, z + 1)].type == NONE) index = CELL_INDEX(x - 1, y - 1, z + 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z + 1)].type == NONE) index = CELL_INDEX(x + 1, y - 1, z + 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z - 1)].type == NONE) index = CELL_INDEX(x + 1, y - 1, z - 1);

				cells[index].type = MAT_SAND;
				cells[index].is_updated = true;
			} break;

			case MAT_WATER:
			{
				cells[index].type = NONE;

				if      (cells[CELL_INDEX(x + 0, y - 1, z + 0)].type == NONE) index = CELL_INDEX(x + 0, y - 1, z + 0);
				else if (cells[CELL_INDEX(x + 0, y - 1, z + 1)].type == NONE) index = CELL_INDEX(x + 0, y - 1, z + 1);
				else if (cells[CELL_INDEX(x + 0, y - 1, z - 1)].type == NONE) index = CELL_INDEX(x + 0, y - 1, z - 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z + 0)].type == NONE) index = CELL_INDEX(x + 1, y - 1, z + 0);
				else if (cells[CELL_INDEX(x - 1, y - 1, z + 0)].type == NONE) index = CELL_INDEX(x - 1, y - 1, z + 0);
				else if (cells[CELL_INDEX(x - 1, y - 1, z - 1)].type == NONE) index = CELL_INDEX(x - 1, y - 1, z - 1);
				else if (cells[CELL_INDEX(x - 1, y - 1, z + 1)].type == NONE) index = CELL_INDEX(x - 1, y - 1, z + 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z + 1)].type == NONE) index = CELL_INDEX(x + 1, y - 1, z + 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z - 1)].type == NONE) index = CELL_INDEX(x + 1, y - 1, z - 1);
				else if (cells[CELL_INDEX(x - 1, y + 0, z - 1)].type == NONE) index = CELL_INDEX(x - 1, y + 0, z - 1);
				else if (cells[CELL_INDEX(x - 1, y + 0, z + 1)].type == NONE) index = CELL_INDEX(x - 1, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 1, y + 0, z + 1)].type == NONE) index = CELL_INDEX(x + 1, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 1, y + 0, z - 1)].type == NONE) index = CELL_INDEX(x + 1, y + 0, z - 1);
				else if (cells[CELL_INDEX(x + 0, y + 0, z + 1)].type == NONE) index = CELL_INDEX(x + 0, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 0, y + 0, z - 1)].type == NONE) index = CELL_INDEX(x + 0, y + 0, z - 1);
				else if (cells[CELL_INDEX(x + 1, y + 0, z + 0)].type == NONE) index = CELL_INDEX(x + 1, y + 0, z + 0);
				else if (cells[CELL_INDEX(x - 1, y + 0, z + 0)].type == NONE) index = CELL_INDEX(x - 1, y + 0, z + 0);

				cells[index].type = MAT_WATER;
				cells[index].is_updated = true;
			} break;

			case MAT_FIRE:
			{
				uint old = index;
				cells[index].type = NONE;

				if      (cells[CELL_INDEX(x + 0, y + 1, z + 0)].type) index = CELL_INDEX(x + 0, y + 1, z + 0);
				else if (cells[CELL_INDEX(x + 0, y + 1, z + 1)].type) index = CELL_INDEX(x + 0, y + 1, z + 1);
				else if (cells[CELL_INDEX(x + 0, y + 1, z - 1)].type) index = CELL_INDEX(x + 0, y + 1, z - 1);
				else if (cells[CELL_INDEX(x + 1, y + 1, z + 0)].type) index = CELL_INDEX(x + 1, y + 1, z + 0);
				else if (cells[CELL_INDEX(x - 1, y + 1, z + 0)].type) index = CELL_INDEX(x - 1, y + 1, z + 0);
				else if (cells[CELL_INDEX(x - 1, y + 1, z - 1)].type) index = CELL_INDEX(x - 1, y + 1, z - 1);
				else if (cells[CELL_INDEX(x - 1, y + 1, z + 1)].type) index = CELL_INDEX(x - 1, y + 1, z + 1);
				else if (cells[CELL_INDEX(x + 1, y + 1, z + 1)].type) index = CELL_INDEX(x + 1, y + 1, z + 1);
				else if (cells[CELL_INDEX(x + 1, y + 1, z - 1)].type) index = CELL_INDEX(x + 1, y + 1, z - 1);

				cells[index].type = MAT_FIRE;
				cells[index].is_updated = true;
				index = old;

				if      (cells[CELL_INDEX(x + 0, y + 0, z + 1)].type) index = CELL_INDEX(x + 0, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 0, y + 0, z - 1)].type) index = CELL_INDEX(x + 0, y + 0, z - 1);
				else if (cells[CELL_INDEX(x + 1, y + 0, z + 0)].type) index = CELL_INDEX(x + 1, y + 0, z + 0);
				else if (cells[CELL_INDEX(x - 1, y + 0, z + 0)].type) index = CELL_INDEX(x - 1, y + 0, z + 0);
				else if (cells[CELL_INDEX(x - 1, y + 0, z - 1)].type) index = CELL_INDEX(x - 1, y + 0, z - 1);
				else if (cells[CELL_INDEX(x - 1, y + 0, z + 1)].type) index = CELL_INDEX(x - 1, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 1, y + 0, z + 1)].type) index = CELL_INDEX(x + 1, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 1, y + 0, z - 1)].type) index = CELL_INDEX(x + 1, y + 0, z - 1);

				cells[index].type = MAT_FIRE;
				cells[index].is_updated = true;
				index = old;

				if      (cells[CELL_INDEX(x + 0, y - 1, z + 0)].type) index = CELL_INDEX(x + 0, y + 0, z + 0);
				else if (cells[CELL_INDEX(x + 0, y - 1, z + 1)].type) index = CELL_INDEX(x + 0, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 0, y - 1, z - 1)].type) index = CELL_INDEX(x + 0, y + 0, z - 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z + 0)].type) index = CELL_INDEX(x + 1, y + 0, z + 0);
				else if (cells[CELL_INDEX(x - 1, y - 1, z + 0)].type) index = CELL_INDEX(x - 1, y + 0, z + 0);
				else if (cells[CELL_INDEX(x - 1, y - 1, z - 1)].type) index = CELL_INDEX(x - 1, y + 0, z - 1);
				else if (cells[CELL_INDEX(x - 1, y - 1, z + 1)].type) index = CELL_INDEX(x - 1, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z + 1)].type) index = CELL_INDEX(x + 1, y + 0, z + 1);
				else if (cells[CELL_INDEX(x + 1, y - 1, z - 1)].type) index = CELL_INDEX(x + 1, y + 0, z - 1);

				cells[index].type = NONE;
				cells[index].is_updated = true;

			} break;
		}
	} } }

	for (uint i = 0; i < NUM_ARENA_CELLS; ++i)
	{
		cells[i].is_updated = false;
	}

	cells[CELL_INDEX(4, 6, 4)].type = MAT_SAND;
}

// rendering

struct Cell_Drawable
{
	vec3 position;
	vec3 color;
};

struct Cell_Renderer
{
	uint num_cells;
	Cell_Drawable cells[NUM_ARENA_CELLS * MAX_CELL_ARENAS];

	Drawable_Mesh cell_mesh;
	Shader cell_shader; // not that kind
};

void init(Cell_Renderer* renderer)
{
	load(&renderer->cell_mesh, "assets/meshes/cell.model", sizeof(renderer->cells));
	mesh_add_attrib_vec3(2, sizeof(Cell_Drawable), 0); // world pos
	mesh_add_attrib_vec3(3, sizeof(Cell_Drawable), sizeof(vec3)); // color
	mesh_add_attrib_vec3(4, sizeof(Cell_Drawable), sizeof(vec3) + sizeof(vec3)); // scale

	load(&(renderer->cell_shader), "assets/shaders/cell.vert", "assets/shaders/cell.frag");
	bind(renderer->cell_shader);
	set_int(renderer->cell_shader, "positions", 0);
	set_int(renderer->cell_shader, "normals"  , 1);
	set_int(renderer->cell_shader, "albedo"   , 2);
}
void update_renderer(Cell_Renderer* renderer, Cell_Arena* arena)
{
	uint num_cells = 0;
	Cell_Drawable* cell_mem = renderer->cells;

	float offset = (1.f / CELL_ARENA_X);

	for (uint x = 0; x < CELL_ARENA_X; ++x) {
	for (uint z = 0; z < CELL_ARENA_Z; ++z) {
	for (uint y = 0; y < CELL_ARENA_Y; ++y)
	{
		uint index = CELL_INDEX(x, y, z);

		switch (arena->cells[index].type)
		{
		case MAT_SAND:
		{
			cell_mem->position = vec3(offset * x, offset * y, offset * z) + vec3(arena->coords);
			cell_mem->color    = vec3(0.83, 0.69, 0.22);

			num_cells++;
			cell_mem++;
		} break;
		case MAT_WATER:
		{
			cell_mem->position = vec3(offset * x, offset * y, offset * z) + vec3(arena->coords);
			cell_mem->color = vec3(0, 1, 1);

			num_cells++;
			cell_mem++;
		} break;

		case MAT_FIRE:
		{
			cell_mem->position = vec3(offset * x, offset * y, offset * z) + vec3(arena->coords);
			cell_mem->color = vec3(.886, .345, .133);

			num_cells++;
			cell_mem++;
		} break;
		}
	} } }

	renderer->num_cells = num_cells;
	update(renderer->cell_mesh, num_cells * sizeof(Cell_Drawable), (byte*)renderer->cells);
}