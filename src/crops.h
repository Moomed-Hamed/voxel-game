#include "blocks.h"

#define MAX_CROPS 16

#define CROP_WHEAT  1
#define CROP_CARROT 2
#define CROP_POTATO 3
#define CROP_OAK_TREE 4

struct Crop
{
	uint  type;
	uvec3 coords;
	uint  yield; // amount of items to drop when broken
	float time_to_maturity; // should this be an int? (if we transitions to ticks)
};

void spawn_crop(Crop* crops, uvec3 pos, uint type)
{
	for (uint i = 0; i < MAX_CROPS; i++)
	{
		if (crops[i].type == NULL)
		{
			crops[i].type = type;
			crops[i].coords = pos;

			switch (type)
			{
			case CROP_WHEAT   : crops[i].time_to_maturity = 10; break;
			case CROP_CARROT  : crops[i].time_to_maturity = 20; break;
			case CROP_POTATO  : crops[i].time_to_maturity = 30; break;
			case CROP_OAK_TREE: crops[i].time_to_maturity = 40; break;
			}

			return;
		}
	}
}
void update(Crop* crops, float dtime)
{
	for (uint i = 0; i < MAX_CROPS; i++)
	{
		if (crops[i].type != NULL)
		{
			if (crops[i].time_to_maturity > 0)
			{
				crops[i].time_to_maturity -= dtime;
			}
			else
			{
				//crops[i] = {};
			}
		}
	}
}

// rendering

struct Crop_Drawable
{
	vec3 position;
	vec3 color;
	vec3 scale;
};

struct Crop_Renderer
{
	uint num_crops;
	Crop_Drawable crops[MAX_CROPS];

	Drawable_Mesh crop_mesh;
	Shader crop_shader;
};

void init(Crop_Renderer* renderer)
{
	load(&renderer->crop_mesh, "assets/meshes/crop.mesh", sizeof(renderer->crops));
	mesh_add_attrib_vec3(2, sizeof(Crop_Drawable), 0); // world pos
	mesh_add_attrib_vec3(3, sizeof(Crop_Drawable), sizeof(vec3)); // color
	mesh_add_attrib_vec3(4, sizeof(Crop_Drawable), sizeof(vec3) + sizeof(vec3)); // scale

	load(&(renderer->crop_shader), "assets/shaders/crop.vert", "assets/shaders/crop.frag");
	bind(renderer->crop_shader);
	set_int(renderer->crop_shader, "positions", 0);
	set_int(renderer->crop_shader, "normals"  , 1);
	set_int(renderer->crop_shader, "albedo"   , 2);
}
void update_renderer(Crop_Renderer* renderer, Crop* crops)
{
	uint num_crops = 0;
	Crop_Drawable* crop_mem = renderer->crops;

	for (int i = 0; i < MAX_CROPS; i++)
	{
		switch (crops[i].type)
		{
		case CROP_WHEAT:
		{
			float time = crops[i].time_to_maturity;

			crop_mem->position = crops[i].coords;
			crop_mem->color = (time < 0) ? vec3(0.83, 0.69, 0.22) : vec3(0.69, 0.83, 0.22);
			crop_mem->scale = (time < 0) ? vec3(1, 1, 1) : vec3(1, 1 - (time / 10), 1);

			num_crops++;
			crop_mem++;
		} break;
		}
	}

	renderer->num_crops = num_crops;
	update(renderer->crop_mesh, num_crops* sizeof(Crop_Drawable), (byte*)renderer->crops);
}