#include "chunk.h"

#define MAX_ASSETS 1

struct Asset
{
	uint type;
	vec3 position;
};

void spawn_asset(Asset* assets, vec3 pos, uint type = 1)
{
	for (uint i = 0; i < MAX_ASSETS; i++)
	{
		if (assets[i].type == NULL)
		{
			assets[i].type = type;
			assets[i].position = pos;
			return;
		}
	}
}

// rendering

struct Asset_Drawable
{
	vec3 position;
	mat3 rotation;
};

struct Asset_Renderer
{
	uint num_assets;
	Asset_Drawable assets[1];

	Drawable_Mesh_UV mesh;
	Shader shader;
};

void init(Asset_Renderer* renderer)
{
	load(&renderer->mesh, "assets/meshes/pickaxe.mesh_uv", "assets/textures/palette.bmp", sizeof(renderer->assets));
	mesh_add_attrib_vec3(3, sizeof(Asset_Drawable), 0); // world pos
	mesh_add_attrib_mat3(4, sizeof(Asset_Drawable), sizeof(vec3)); // rotation

	load(&(renderer->shader), "assets/shaders/mesh_uv_rot.vert", "assets/shaders/mesh_uv.frag");
	bind(renderer->shader);
	set_int(renderer->shader, "positions", 0);
	set_int(renderer->shader, "normals"  , 1);
	set_int(renderer->shader, "albedo"   , 2);
	set_int(renderer->shader, "texture_sampler", 4);
}
void update_renderer(Asset_Renderer* renderer, vec3 pos, vec3 up, vec3 front)
{
	Asset_Drawable* asset_mem = renderer->assets;

	asset_mem->position = pos;
	asset_mem->rotation = point_at(-1.f * front, up);

	renderer->num_assets = 1;
	update(renderer->mesh, sizeof(Asset_Drawable), (byte*)renderer->assets);
}