#include "asset.h"

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

	Chunk_Renderer renderers[9];
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

	for (uint i = 0; i < 9; i++) { init(world->renderers + i); }
}

void update(World* world, vec3 position)
{
	uint chunk_x =  (((uint)position.x) & 0xFFF0 ) - 16;
	uint chunk_z =  (((uint)position.z) & 0xFFF0 ) - 16;

	static uvec2 last_coords = uvec2(0, 0);
	
	// rn i reload everything if we move to a new chunk
	// later i'll move unloaded chunks to border chunks &
	// only generate what we need
	if (last_coords != uvec2(chunk_x, chunk_z))
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
	}

	last_coords = uvec2(chunk_x, chunk_z);
}

// rendering

void update_renderer(World* world, float dtime)
{
	for (uint i = 0; i < 9; i++)
	{
		update_renderer(world->renderers + i, world->active_chunks + i);
	}
}

void draw(World* world, float dtime, mat4 proj_view)
{
	static float timer = 0; timer += .25 * dtime;

	for (uint i = 0; i < 9; i++)
	{
		bind(world->renderers[i].solid_shader);
		set_mat4(world->renderers[i].solid_shader, "proj_view", proj_view);
		bind_texture(world->renderers[i].solid_mesh, 3);

		draw(world->renderers[i].solid_mesh, world->renderers[i].num_solids);
		
		bind(world->renderers[i].fluid_shader);
		set_mat4(world->renderers[i].fluid_shader, "proj_view", proj_view);
		set_float(world->renderers[i].fluid_shader, "timer", timer);

		draw(world->renderers[i].fluid_mesh, world->renderers[i].num_fluids);
	}
}

void world_set_block(Chunk* chunks, vec3 pos, BlockID new_block)
{
	uint chunk_x = ((uint)pos.x) & 0xFFF0;
	uint chunk_z = ((uint)pos.z) & 0xFFF0;

	uint local_x = (uint)pos.x - chunk_x;
	uint local_z = (uint)pos.z - chunk_z;
	uint local_y = (uint)pos.y;

	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++)
	{
		if (chunks[i].coords == uvec2(chunk_x, chunk_z))
		{
			chunks[i].blocks[CHUNK_BLOCK_INDEX(local_x, local_y, local_z)] = new_block;
			return;
		}
	}
}
void world_set_block(Chunk* chunks, uvec3 coords, BlockID new_block)
{
	uint chunk_x = coords.x & 0xFFF0;
	uint chunk_z = coords.z & 0xFFF0;

	uint local_x = coords.x - chunk_x;
	uint local_z = coords.z - chunk_z;
	uint local_y = coords.y;

	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++)
	{
		if (chunks[i].coords == uvec2(chunk_x, chunk_z))
		{
			chunks[i].blocks[CHUNK_BLOCK_INDEX(local_x, local_y, local_z)] = new_block;
			return;
		}
	}
}
BlockID world_get_block(Chunk* chunks, vec3 pos)
{
	uint chunk_x = ((uint)pos.x) & 0xFFF0;
	uint chunk_z = ((uint)pos.z) & 0xFFF0;

	uint local_x = (uint)pos.x - chunk_x;
	uint local_z = (uint)pos.z - chunk_z;
	uint local_y = (uint)pos.y;

	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++)
	{
		if (chunks[i].coords == uvec2(chunk_x, chunk_z))
		{
			return chunks[i].blocks[CHUNK_BLOCK_INDEX(local_x, local_y, local_z)];
		}
	}

	return INVALID;
}
BlockID world_get_block_raycast(Chunk* chunks, vec3 pos, vec3 dir)
{
	vec3 increment = vec3(.2, .2, .2) * glm::normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 3.6f)
	{
		BlockID block = world_get_block(chunks, test_point);
		if (block) return block;

		total_distance += increment_length;
		test_point += increment;
	}

	return INVALID;
}
BlockID world_break_block_raycast(Chunk* chunks, vec3 pos, vec3 dir, vec3* breakpos = NULL)
{
	vec3 increment = vec3(.2, .2, .2) * glm::normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 3.6f)
	{
		BlockID test_block = world_get_block(chunks, test_point);

		if (test_block)
		{
			world_set_block(chunks, test_point, BLOCK_AIR);

			if (breakpos) // aaah! if statement slow! slow bad!
			{
				*breakpos = test_point;
				breakpos->y = ceil(breakpos->y);
			}

			return test_block;
		}

		total_distance += increment_length;
		test_point += increment;
	}

	return BLOCK_AIR;
}
BlockID world_place_block_raycast(Chunk* chunks, vec3 pos, vec3 dir, BlockID block, uvec3* place_pos = NULL)
{
	vec3 increment = vec3(.2, .2, .2) * glm::normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 4)
	{
		BlockID test_block = world_get_block(chunks, test_point);

		if (test_block && test_block != INVALID)
		{
			world_set_block(chunks, test_point - increment, block);
			if(place_pos) *place_pos = uvec3(test_point - increment);
			return test_block;
		}

		total_distance += increment_length;
		test_point += increment;
	}

	return BLOCK_AIR;
}
void world_get_place_pos_raycast(Chunk* chunks, vec3 pos, vec3 dir, uvec3* place_pos)
{
	vec3 increment = vec3(.2, .2, .2) * glm::normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 4)
	{
		if (world_get_block(chunks, test_point) && world_get_block(chunks, test_point) != INVALID)
		{
			*place_pos = uvec3(test_point - increment); // position of where a block *would* be placed
			return;
		}

		total_distance += increment_length;
		test_point += increment;
	}

	place_pos->x = INVALID;
}