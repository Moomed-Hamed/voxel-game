#include "items.h"

#define MAX_WORLD_ITEMS 64 // items that can be dropped in the world

extern uint give_player_item(Item* player_items, Item item); // is this bad?

struct World_Item // an item that has been dropped in the world
{
	Item item;
	vec3 position;
	vec3 velocity;
};

void spawn(World_Item* items, Item item, vec3 position)
{
	for (uint i = 0; i < MAX_WORLD_ITEMS; i++)
	{
		if (items[i].item.type == NULL)
		{
			items[i].item = item;
			items[i].position = position;
			items[i].velocity = {0, 1, 0};
			return;
		}
	}
}

struct World
{
	Chunk_Loader chunks;
	World_Item items[MAX_WORLD_ITEMS];
};

void init(World* world, vec3 position)
{
	// without this, all chunks will have blocks_index = 0
	for (uint i = 0; i < NUM_CHUNKS; i++)
		world->chunks.loaded_chunks[i].blocks_index = i;
}
void update(World* world, Camera camera, Mouse mouse, float dtime, Item* player_items, Audio* pops)
{
	update_chunks(&world->chunks, camera.position);

	// world item physics
	World_Item* items = world->items;
	for (uint i = 0; i < MAX_WORLD_ITEMS; i++)
	{
		if (items[i].item.type > 0)
		{
			vec3 dir = camera.position - items[i].position;
			float distance_to_player = length(dir);

			if (distance_to_player < 2)
			{
				play_audio(pops[random_uint() % 3]);
				if(give_player_item(player_items, items[i].item))
					items[i] = {};
			}
			else
			{
				if (distance_to_player < 4) // this should be else if
					items[i].velocity += normalize(dir) * dtime * 2.f;
				else
					items[i].velocity *= vec3(0, 1, 0);

				vec3 pos = items[i].position;
				vec3 vel = items[i].velocity;

				pos += vel * dtime;

				if (get_block(&world->chunks, pos))
				{
					//if(ceil(pos.x) - pos.x > floor(pos.x) - pos.x)

					//vel.x = (ceil(pos.x) - pos.x) * 30; // bounce force
					vel.y = (ceil(pos.y) - pos.y) * 30; // bounce force
					//vel.z = (ceil(pos.z) - pos.z) * 30; // bounce force
				}
				else
					vel.y += GRAVITY * dtime;

				items[i].position = pos;
				items[i].velocity = vel;
			}
		}
	}
}

// rendering

struct Item_Drawable
{
	vec3 position;
	vec2 tex_offset;
};

struct World_Renderer
{
	GLuint texture, material;

	// terrain
	Shader solid_shader, fluid_shader;
	Chunk_Renderer chunks[NUM_ACTIVE_CHUNKS];

	// world items
	uint num_blocks;
	Item_Drawable blocks[MAX_WORLD_ITEMS]; // just for blocks
	Item_Drawable items [MAX_WORLD_ITEMS]; // general purpose
	Drawable_Mesh_UV block_mesh, item_mesh;
	Shader block_shader;
	mat3 transform;
};

void init(World_Renderer* renderer)
{
	renderer->texture  = load_texture("assets/textures/block_atlas.bmp");
	renderer->material = load_texture("assets/textures/materials.bmp"  );

	// terrain
	for (uint i = 0; i < 9; i++)
		init(renderer->chunks + i);

	load(&renderer->solid_shader, "assets/shaders/chunk/solid.vert", "assets/shaders/mesh_uv.frag");
	load(&renderer->fluid_shader, "assets/shaders/chunk/fluid.vert", "assets/shaders/mesh.frag");

	// world items
	load(&renderer->block_mesh, "assets/meshes/block.mesh_uv", sizeof(renderer->blocks));
	mesh_add_attrib_vec3 (3, sizeof(Item_Drawable), 0); // world position
	mesh_add_attrib_float(4, sizeof(Item_Drawable), sizeof(vec3)); // texture offset

	load(&renderer->block_shader, "assets/shaders/chunk/item.vert", "assets/shaders/mesh_uv.frag");
}
void update(World_Renderer* renderer, World* world, float dtime)
{
	// terrain
	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++) // TODO : we don't need to render chunks the player can't see
		update(renderer->chunks + i, world->chunks.active[i], world->chunks.blocks);

	// world items
	static float timer = 0; timer = (timer > TWOPI) ? 0 : timer + (TWOPI * dtime) / 5;
	float offset = .05f + (sinf(timer * 2.f) * .05f);

	renderer->transform = mat3(.25) * mat3(rotate(timer, vec3(0, 1, 0)));

	World_Item* items = world->items;

	uint num_blocks = 0;
	for (uint i = 0; i < MAX_WORLD_ITEMS; i++)
	{
		switch (items[i].item.type)
		{
		case ITEM_BLOCK : {
			renderer->blocks[num_blocks].position = items[i].position + vec3(0, offset, 0);
			renderer->blocks[num_blocks++].tex_offset = vec2(items[i].item.id - 1.f, 0) / vec2(16);
		} break;
		}
	}

	renderer->num_blocks = num_blocks;
	update(renderer->block_mesh, num_blocks * sizeof(Item_Drawable), (byte*)renderer->blocks);
}
void draw(World_Renderer* renderer, mat4 proj_view, float dtime)
{
	// terrain
	bind(renderer->solid_shader);
	set_mat4(renderer->solid_shader, "proj_view", proj_view);
	bind_texture(renderer->texture, 0);
	bind_texture(renderer->material, 1);

	for (uint i = 0; i < 9; i++) // draw solids
		draw(renderer->chunks[i].solid_mesh, renderer->chunks[i].num_solids);

	static float timer = 0; timer += .25f * dtime;
	bind(renderer->fluid_shader);
	set_mat4(renderer->fluid_shader, "proj_view", proj_view);
	set_float(renderer->fluid_shader, "timer", timer);

	for (uint i = 0; i < 9; i++) // draw fluids
		draw(renderer->chunks[i].fluid_mesh, renderer->chunks[i].num_fluids);

	// world items
	bind(renderer->block_shader);
	set_mat4(renderer->block_shader, "proj_view", proj_view);
	set_mat3(renderer->block_shader, "transform", renderer->transform);
	bind_texture(renderer->texture , 0);
	bind_texture(renderer->material, 1);
	draw(renderer->block_mesh, renderer->num_blocks);
}