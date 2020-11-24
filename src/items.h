#include "farming.h"

#define GRAVITY -.9f

#define MAX_LOOSE_ITEMS 256

#define BLOCK_ITEM 1

struct Item
{
	uint16 main_type;
	uint16 sub_type;
	uint16 count;
	uint16 index;
};

struct Loose_Item : Item
{
	vec3 position, velocity;
};

void add_loose_item(Loose_Item* items, Item item, vec3 position, vec3 velocity)
{
	for (int i = 0; i < MAX_LOOSE_ITEMS; i++)
	{
		if (items[i].main_type == NONE)
		{
			items[i].main_type = item.main_type;
			items[i].sub_type  = item.sub_type;
			items[i].count     = item.count;
			items[i].index     = item.index;
			items[i].position  = position;
			items[i].velocity  = velocity;
			return;
		}
	}
}
void update(Loose_Item* items, Chunk* chunks, vec3 player_pos, float frame_time)
{
	for (int i = 0; i < MAX_LOOSE_ITEMS; i++)
	{
		if (items[i].main_type != NONE)
		{
			vec3 dir = player_pos - items[i].position;
			float distance_to_player = glm::length(dir);
 			
			//if (distance_to_player < 1.5)
			//	player_pickup_item
			if (distance_to_player < 4) // this should be else if
				items[i].velocity += glm::normalize(dir) * 25.f * frame_time;
			else
				items[i].velocity *= vec3(0, 1, 0);

			vec3 pos = items[i].position;
			vec3 vel = items[i].velocity;

			if (pos.y < 1) pos.y = 1;

			pos += vel * frame_time;

			if (world_get_block(chunks, pos))
				vel.y = (ceil(pos.y) - pos.y) * 30; // bounce force
			else
				vel.y += GRAVITY * .4;

			items[i].position = pos;
			items[i].velocity = vel;
		}
	}
}

// -- rendering -- //

struct Block_Item_Drawable { vec3 position; vec2 tex_offset; };

struct Loose_Item_Renderer
{
	uint num_blocks;

	Block_Item_Drawable blocks[MAX_LOOSE_ITEMS];

	Drawable_Mesh_UV block_mesh;
	Shader block_shader;
};

void init(Loose_Item_Renderer* renderer)
{
	// blocks
	load(&renderer->block_mesh, "assets/meshes/block.model", "assets/textures/texture_atlas.bmp", sizeof(renderer->blocks));
	mesh_add_attrib_vec3(3, sizeof(Block_Item_Drawable), 0); // world pos
	mesh_add_attrib_vec2(4, sizeof(Block_Item_Drawable), sizeof(vec3)); // tex coord

	load(&(renderer->block_shader), "assets/shaders/dropped_item.vert", "assets/shaders/solid.frag");
	bind(renderer->block_shader);
	set_int(renderer->block_shader, "positions", 0);
	set_int(renderer->block_shader, "normals"  , 1);
	set_int(renderer->block_shader, "albedo"   , 2);
	set_int(renderer->block_shader, "texture_sampler", 3);
}
void update_renderer(Loose_Item_Renderer* renderer, Loose_Item* items, float frame_time)
{
	uint num_blocks = 0;
	Block_Item_Drawable* block_mem = renderer->blocks;

	// used to give items a floating effect
	static float periodic_timer = 0;
	static float y_offset = 0;

	periodic_timer += 2.f * frame_time;
	if (periodic_timer > TWOPI) periodic_timer = 0;
	y_offset = sin(periodic_timer);

	for (int i = 0; i < MAX_LOOSE_ITEMS; i++)
	{
		switch (items[i].main_type)
		{
		case BLOCK_ITEM :
		{
			block_mem->position = items[i].position + vec3{ 0, .3 + (.15 * y_offset), 0 };
			block_mem->tex_offset = block_get_tex_offset(items[i].sub_type);
			num_blocks++;
			block_mem++;
		} break;
		}
	}

	renderer->num_blocks = num_blocks;
	update(renderer->block_mesh, num_blocks * sizeof(Block_Item_Drawable), (byte*)renderer->blocks);
}