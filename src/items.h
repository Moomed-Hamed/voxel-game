#include "enemy.h"

#define MAX_ITEMS 16

#define ITEM_BLOCK 1

struct Item
{
	uint type, id;
	Cube_Collider_AA collider;
};

void spawn(Item* items, uint type, uint id, vec3 position)
{
	for (uint i = 0; i < MAX_ITEMS; i++)
	{
		if (items[i].type == NULL)
		{
			items[i].type = type;
			items[i].id = id;
			items[i].collider = {};
			items[i].collider.position = position;
			items[i].collider.scale = vec3(.25);
			return;
		}
	}
}
void update(Item* items, Chunk* chunks, Camera camera, float dtime, Audio* pops)
{
	for (uint i = 0; i < MAX_ITEMS; i++)
	{
		if (items[i].type > 0)
		{
			vec3 dir = camera.position - items[i].collider.position;
			float distance_to_player = length(dir);

			if (distance_to_player < 2)
			{
				play_audio(pops[random_int() % 3]);
				items[i] = {};
			}
			else
			{
				if (distance_to_player < 4) // this should be else if
					items[i].collider.velocity += normalize(dir) * dtime * 2.f;
				else
					items[i].collider.velocity *= vec3(0, 1, 0);

				vec3 pos = items[i].collider.position;
				vec3 vel = items[i].collider.velocity;

				pos += vel * dtime;

				if (get_block(chunks, pos))
				{
					//if(ceil(pos.x) - pos.x > floor(pos.x) - pos.x)

					//vel.x = (ceil(pos.x) - pos.x) * 30; // bounce force
					vel.y = (ceil(pos.y) - pos.y) * 30; // bounce force
					//vel.z = (ceil(pos.z) - pos.z) * 30; // bounce force
				}
				else
					vel.y += GRAVITY * dtime;

				items[i].collider.position = pos;
				items[i].collider.velocity = vel;
			}
		}
	}
}

// rendering

struct Block_Item_Drawable
{
	vec3 position;
	mat3 transform;
	float tex_offset;
};

struct Item_Renderer
{
	uint num_blocks;
	Block_Item_Drawable blocks[MAX_ITEMS];
	Drawable_Mesh_UV mesh;
	Shader shader;
};

void init(Item_Renderer* renderer)
{
	load(&renderer->mesh, "assets/meshes/block.mesh_uv", sizeof(renderer->blocks));
	mesh_add_attrib_vec3 (3, sizeof(Block_Item_Drawable), 0); // position
	mesh_add_attrib_mat3 (4, sizeof(Block_Item_Drawable), sizeof(vec3)); // transform
	mesh_add_attrib_float(7, sizeof(Block_Item_Drawable), sizeof(vec3) + sizeof(mat3)); // texture offset

	renderer->mesh.texture_id  = load_texture("assets/textures/block_atlas.bmp");
	renderer->mesh.material_id = load_texture("assets/textures/atlas_mat.bmp");
	load(&(renderer->shader), "assets/shaders/item.vert", "assets/shaders/mesh_uv.frag");
}
void update_renderer(Item_Renderer* renderer, Item* items, float dtime)
{
	static float timer = 0; timer += dtime / 5;
	if (timer > 1) timer = 0;
	float offset = .05 + (sin(timer * 2 * TWOPI) * .05f);

	uint num_blocks = 0;
	for (uint i = 0; i < MAX_ITEMS; i++)
	{
		switch (items[i].type)
		{
		case ITEM_BLOCK : {
			renderer->blocks[num_blocks].position = items[i].collider.position + vec3(0, offset, 0);
			renderer->blocks[num_blocks].transform = mat3(.25) * mat3(rotate(TWOPI * timer, vec3(0, 1, 0)));
			renderer->blocks[num_blocks++].tex_offset = (float)(items[i].id - 1) * (1 / 16.f);
		} break;
		}
	}

	renderer->num_blocks = num_blocks;
	update(renderer->mesh, num_blocks * sizeof(Block_Item_Drawable), (byte*)renderer->blocks);
}
void draw(Item_Renderer* renderer, mat4 proj_view)
{
	bind(renderer->shader);
	set_mat4(renderer->shader, "proj_view", proj_view);
	bind_texture(renderer->mesh);
	draw(renderer->mesh, renderer->num_blocks);
}