#include "tools.h"

struct World
{
	union
	{
		Chunk chunks[9 + 9];
	
		struct
		{
			Chunk active_chunks[9];
			Chunk border_chunks[9];
		};
	};

	Audio block_break;
};

void init(World* world, vec3 position)
{
	uint offset_x = ( ((uint)position.x) & 0xFFF0 ) - 16;
	uint offset_z = ( ((uint)position.z) & 0xFFF0 ) - 16;

	uint i = 0;
	for (uint x = 0; x < 3; ++x) {
	for (uint z = 0; z < 3; ++z)
	{
		uvec2 coords = uvec2((x * CHUNK_X) + offset_x, (z * CHUNK_Z) + offset_z);
		world->chunks[i].coords = coords;
		generate_chunk_terrain(&world->chunks[i]);
		i++;
	} }

	//audio
	world->block_break = load_audio("assets/audio/block.audio");
}
void update(World* world, Keyboard keys, Mouse mouse, Camera camera, Particle_Emitter* emitter, Item* items)
{
	vec3 position = camera.position;
	vec3 front    = camera.front;

	//  --- chunk updates --- //
	uint chunk_x =  (((uint)position.x) & 0xFFF0 ) - 16;
	uint chunk_z =  (((uint)position.z) & 0xFFF0 ) - 16;
	
	// rn i reload everything if we move to a new chunk
	// later i'll move unloaded chunks to border chunks & only generate what we need

	//uvec2 loaded_chunks[9] = {}; // chunks that are currently loaded
	//for (uint i = 0; i < 9; i++)
	//{
	//	loaded_chunks[i] = world->active_chunks[i].coords;
	//}

	static uvec2 last_coords = {}; // last frame's player chunk
	if (last_coords != uvec2(chunk_x, chunk_z)) // player is in a new chunk
	{
		uint offset_x = ( ((uint)position.x) & 0xFFF0 ) - 16;
		uint offset_z = ( ((uint)position.z) & 0xFFF0 ) - 16;

		uint i = 0;
		for (uint x = 0; x < 3; ++x) {
		for (uint z = 0; z < 3; ++z)
		{
			uvec2 coords = uvec2((x * CHUNK_X) + offset_x, (z * CHUNK_Z) + offset_z);
			world->active_chunks[i].coords = coords;
			generate_chunk_terrain(&world->active_chunks[i]);
			i++;
		} }
	} last_coords = uvec2(chunk_x, chunk_z);

	// block break/place
	if (mouse.left_button.is_pressed && !mouse.left_button.was_pressed)
	{
		vec3 break_pos = {};
		BlockID block = break_block_raycast(world->chunks, position, front, &break_pos);
		if (block)
		{
			spawn_blockbreak(emitter, break_pos);
			spawn(items, ITEM_BLOCK, block, vec3(break_pos.x, break_pos.y, break_pos.z));
			play_audio(world->block_break);
		}
	}
	if (mouse.right_button.is_pressed && !mouse.right_button.was_pressed)
	{
		uvec3 place_pos = {};
		BlockID block = place_block_raycast(world->chunks, position, front, BLOCK_DIRT, &place_pos);
		if (block) { }
	}
}

// rendering

struct World_Renderer
{
	Chunk_Renderer chunks[9];
};

void init(World_Renderer* renderer)
{
	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++) { init(renderer->chunks + i); }
}
void update_renderer(World_Renderer* renderer, World* world, float dtime)
{
	for (uint i = 0; i < 9; i++)
	{
		update_renderer(renderer->chunks + i, world->active_chunks + i);
	}
}
void draw(World_Renderer* renderer, float dtime, mat4 proj_view)
{
	static float timer = 0; timer += .25f * dtime;

	for (uint i = 0; i < 9; i++)
	{
		draw(renderer->chunks + i, proj_view, timer);
	}
}

// sky rendering

struct Sky_Drawable
{
	vec3 position;
	mat3 rotation;
};

struct Sky_Renderer
{
	Drawable_Mesh_UV mesh;
	Shader shader;
};

void init(Sky_Renderer* renderer)
{
	load(&renderer->mesh, "assets/meshes/sky.mesh_uv", sizeof(Sky_Drawable));
	mesh_add_attrib_vec3(3, sizeof(Sky_Drawable), 0); // world pos
	mesh_add_attrib_mat3(4, sizeof(Sky_Drawable), sizeof(vec3)); // rotation

	GLuint texture_id  = load_texture("assets/textures/palette.bmp");
	GLuint material_id = load_texture("assets/textures/materials.bmp");

	renderer->mesh.texture_id  = texture_id;
	renderer->mesh.material_id = material_id;

	load(&(renderer->shader), "assets/shaders/transform/mesh_uv.vert", "assets/shaders/mesh_uv.frag");
}
void update_renderer(Sky_Renderer* renderer, vec3 pos)
{
	Sky_Drawable sky = {};
	sky.position = vec3(pos.x, -20, pos.z);
	sky.rotation = mat3(1);

	update(renderer->mesh, sizeof(Sky_Drawable), (byte*)&sky);
}
void draw(Sky_Renderer* renderer, mat4 proj_view)
{
	bind(renderer->shader);
	set_mat4(renderer->shader, "proj_view", proj_view);
	bind_texture(renderer->mesh);
	draw(renderer->mesh);
}