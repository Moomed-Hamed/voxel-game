#include "particles.h"

#define NUM_CHUNKS (7 * 7)
#define NUM_ACTIVE_CHUNKS (3 * 3)
#define NUM_BORDER_CHUNKS ((5 * 5) - NUM_ACTIVE_CHUNKS)
#define NUM_PRIMED_CHUNKS ((7 * 7) - (5 * 5))

#define CHUNK_X 16
#define CHUNK_Z 16
#define CHUNK_Y 128

#define NUM_CHUNK_BLOCKS (CHUNK_X * CHUNK_Z * CHUNK_Y)
#define BLOCK_INDEX(x,y,z,i) ((((x) + (CHUNK_X * (z))) + ((CHUNK_X * CHUNK_Z) * (y))) + (NUM_CHUNK_BLOCKS * i))

#define BLOCK_AIR	0

// symmetrical blocks
#define BLOCK_STONE	1
#define BLOCK_DIRT	2
#define BLOCK_GRASS	3
#define BLOCK_SAND	4
#define BLOCK_WOOD	5
#define BLOCK_BRICK	6
#define BLOCK_STONE_BRICK	7
#define BLOCK_LEAF
#define BLOCK_IRON	
#define BLOCK_COPPER

// ores
#define BLOCK_COAL_ORE		9
#define BLOCK_IRON_ORE		10
#define BLOCK_COPPER_ORE	11
#define BLOCK_DIAMOND_ORE	12
#define BLOCK_EMERALD_ORE	13
#define BLOCK_GOLD_ORE		14
#define BLOCK_RUBY_ORE		15

// multi-face blocks
#define BLOCK_GRASS	3
#define BLOCK_BARK	8

// mechanical / non-electric
#define BLOCK_CRAFTING	20
#define BLOCK_FURNACE	21

// electric
#define BLOCK_QUARRY		22
#define BLOCK_RECYCLER	23
#define BLOCK_CRUSHER	24
#define BLOCK_WASHER		25
#define BLOCK_SMELTER	26
#define BLOCK_GENERATOR	27
#define BLOCK_PUMP			28
#define BLOCK_SOLAR_PANEL	29
#define BLOCK_ASSEMBLER		30
#define BLOCK_OIL_REFINERY	31

// storage
#define BLOCK_CHEST	32
#define BLOCK_TANK	33

// transport
#define BLOCK_PIPE 40
#define BLOCK_BELT 41
#define BLOCK_WIRE 42

// fluids
#define BLOCK_WATER	50
#define BLOCK_WATER	51
#define BLOCK_WATER	52
#define BLOCK_WATER_FLOW	53
#define BLOCK_WATER_FLOW	54
#define BLOCK_WATER_FLOW	55

struct Chunk
{
	union { struct { uint32 x, z; }; uint64 id; uvec2 coords; };
	u16 blocks_index;
};

struct Chunk_Loader
{
	union
	{
		Chunk loaded_chunks[NUM_CHUNKS];

		struct
		{
			Chunk active[NUM_ACTIVE_CHUNKS];
			Chunk border[NUM_BORDER_CHUNKS];
			Chunk primed[NUM_PRIMED_CHUNKS];
		};
	};

	u16 blocks[NUM_CHUNKS * NUM_CHUNK_BLOCKS];
};

float terrain_noise(float x, float y, float scale)
{
	uint octaves = 4;
	float lacunarity = 2; // increase in frequency of octaves
	float persistance = .5; // decrease in amplitude of octaves

	float frequency = 1;
	float amplitude = .5; // don't want result to be > 1

	float n = 0;

	for (uint i = 0; i < octaves; i++)
	{
		n += amplitude * perlin(x / scale * frequency, y / scale * frequency);

		amplitude *= persistance;
		frequency *= lacunarity;
	}

	return n;
}
void generate(Chunk chunk, u16* blocks, float scale = 45)
{
	uint x_pos = chunk.x & 0xFFF0;
	uint z_pos = chunk.z & 0xFFF0;

	uint water_level = 24;

	for (uint x = 0; x < CHUNK_X; ++x) {
	for (uint z = 0; z < CHUNK_Z; ++z)
	{
		vec2 point = vec2{ x + x_pos, z + z_pos } / scale;

		// heightfield noise
		uint height = 64 * terrain_noise(point.x, point.y, 1);

		//// tree noise
		//float n1 = perlin(point.x, point.y);
		//float n2 = perlin(point.x + EPSILON, point.y + EPSILON);
		//float n3 = perlin(point.x + EPSILON2, point.y + EPSILON2);
		//
		//float d1 = (n2 - n1) / EPSILON; // first derivative
		//float d2 = (n2 - (2.f * n1) + n3) / EPSILON2; // second derivative

		for (uint y = 0; y < CHUNK_Y; ++y)
		{
			uint index = BLOCK_INDEX(x, y, z, chunk.blocks_index);

			if (y == water_level && height <= water_level)
			{
				blocks[index] = BLOCK_WATER;
				continue;
			}

			if (y > height)
			{
				blocks[index] = BLOCK_AIR;
				continue;
			}

			if (height > water_level) // land
			{
				if (y == water_level + 1)
					blocks[index] = BLOCK_SAND;
				else if (y == height)
					blocks[index] = BLOCK_GRASS;
				else if (y == height - 1)
					blocks[index] = BLOCK_DIRT;
			}
			else if (y < water_level)// ocean
			{
				if (y == height)
					blocks[index] = BLOCK_WATER;
				else if (y == height - 1)
					blocks[index] = BLOCK_SAND;
			}

			if (y < height - 1) // underground
			{
				blocks[index] = BLOCK_STONE;
			}

			//// trees
			//if (d2 == 0.f && d1 == 0 && height == y)
			//{
			//	blocks[index] = BLOCK_WOOD;
			//	continue;
			//}
		}
	} }
}

uint max(uint a, uint b) { return (a > b) ? a : b; }
uint absi(int a) { return a >= 0 ? a : a * -1; }

void unload_blocks(u16* blocks, uint index)
{
	memset(&blocks[index * NUM_CHUNK_BLOCKS], 0, sizeof(u16) * NUM_CHUNK_BLOCKS);
}
void update_chunks(Chunk_Loader* world, vec3 position)
{
	Chunk* old_chunks = world->loaded_chunks;

	union {
		Chunk loaded[NUM_CHUNKS];

		struct {
			Chunk active[NUM_ACTIVE_CHUNKS];
			Chunk border[NUM_BORDER_CHUNKS];
			Chunk primed[NUM_PRIMED_CHUNKS];
		};
	} new_chunks = {};

	// coords of the chunk containing 'position' divided by chunk dimensions
	int px = ( ((uint)position.x) & 0xFFF0 ) / CHUNK_X;
	int pz = ( ((uint)position.z) & 0xFFF0 ) / CHUNK_Z;

	// build array of new chunks
	uint num_active = 0, num_border = 0, num_primed = 0;
	for (int x = px - 3; x <= px + 3; x++) {
	for (int z = pz - 3; z <= pz + 3; z++)
	{
		uint layer = max(absi(x - px), absi(z - pz));

		if (layer < 2)
			new_chunks.active[num_active++].coords = { x * CHUNK_X, z * CHUNK_Z };
		else if (layer == 2)
			new_chunks.border[num_border++].coords = { x * CHUNK_X, z * CHUNK_Z };
		else
			new_chunks.primed[num_primed++].coords = { x * CHUNK_X, z * CHUNK_Z };

		// move player chunk to position 0 in new_chunks array
		Chunk temp = new_chunks.active[4];
		new_chunks.active[4] = new_chunks.active[0];
		new_chunks.active[0] = temp;
	} }

	assert(num_active == NUM_ACTIVE_CHUNKS);
	assert(num_border == NUM_BORDER_CHUNKS);
	assert(num_primed == NUM_PRIMED_CHUNKS);

	// keep track of block data that has been unloaded
	uint num_free = 0;
	u16 free_blocks[NUM_CHUNKS] = {};

	// check for chunks that need to be unloaded
	for (uint i = 0; i < NUM_CHUNKS; i++) // for each old chunk
	{
		bool keep = false;

		// check if the chunk should stay loaded
		for (uint j = 0; j < NUM_CHUNKS; j++)
		{
			if (old_chunks[i].id == new_chunks.loaded[j].id) // keep this chunk
			{
				keep = true;
				new_chunks.loaded[j] = old_chunks[i];
				break;
			}
		}

		if (keep == false) // chunk should be unloaded
		{
			// mark it as free
			free_blocks[num_free++] = old_chunks[i].blocks_index;

			// unload the chunk data
			unload_blocks(world->blocks, old_chunks[i].blocks_index);
		}
	}

	// check for chunks that need to be generated / loaded from disk
	for (uint i = 0; i < NUM_CHUNKS; i++) // for each new chunk
	{
		bool load = true;

		// check if the chunk is already loaded
		for (uint j = 0; j < NUM_CHUNKS; j++)
		{
			if (new_chunks.loaded[i].id == old_chunks[j].id)
			{
				load = false; // chunk is already loaded
				new_chunks.loaded[i] = old_chunks[j];
				break;
			}
		}

		if (load)
		{
			new_chunks.loaded[i].blocks_index = free_blocks[--num_free];
			generate(new_chunks.loaded[i], world->blocks);
			// TODO : check if it is on disk & load the changes if it is
		}
	}

	// finalizing
	for (uint i = 0; i < NUM_CHUNKS; i++)
		old_chunks[i] = new_chunks.loaded[i];

	assert(num_free == 0);
}

// utilities

void set_block(Chunk_Loader* chunks, vec3 pos, u16 new_block)
{
	uint chunk_x = ((uint)pos.x) & 0xFFF0;
	uint chunk_z = ((uint)pos.z) & 0xFFF0;

	uint local_x = (uint)pos.x - chunk_x;
	uint local_z = (uint)pos.z - chunk_z;
	uint local_y = (uint)pos.y;

	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++)
	{
		if (chunks->active[i].coords == uvec2(chunk_x, chunk_z))
		{
			chunks->blocks[BLOCK_INDEX(local_x, local_y, local_z, chunks->active[i].blocks_index)] = new_block;
			return;
		}
	}
}
void set_block(Chunk_Loader* chunks, uvec3 coords, u16 new_block)
{
	uint chunk_x = coords.x & 0xFFF0;
	uint chunk_z = coords.z & 0xFFF0;

	uint local_x = coords.x - chunk_x;
	uint local_z = coords.z - chunk_z;
	uint local_y = coords.y;

	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++)
	{
		if (chunks->active[i].coords == uvec2(chunk_x, chunk_z))
		{
			chunks->blocks[BLOCK_INDEX(local_x, local_y, local_z, chunks->active[i].blocks_index)] = new_block;
			return;
		}
	}
}
u16 get_block(Chunk_Loader* chunks, vec3 pos)
{
	uint chunk_x = ((uint)pos.x) & 0xFFF0;
	uint chunk_z = ((uint)pos.z) & 0xFFF0;

	uint local_x = (uint)pos.x - chunk_x;
	uint local_z = (uint)pos.z - chunk_z;
	uint local_y = (uint)pos.y;

	for (uint i = 0; i < NUM_ACTIVE_CHUNKS; i++)
		if (chunks->active[i].coords == uvec2(chunk_x, chunk_z))
			return chunks->blocks[BLOCK_INDEX(local_x, local_y, local_z, chunks->active[i].blocks_index)];

	return INVALID;
}
u16 get_block_raycast(Chunk_Loader* chunks, vec3 pos, vec3 dir)
{
	vec3 increment = vec3(.2, .2, .2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = length(increment);
	float total_distance = increment_length;

	while (total_distance < 3.6f)
	{
		u16 block = get_block(chunks, test_point);
		if (block) return block;

		total_distance += increment_length;
		test_point += increment;
	}

	return INVALID;
}
u16 break_block_raycast(Chunk_Loader* chunks, vec3 pos, vec3 dir, vec3* breakpos = NULL)
{
	vec3 increment = vec3(.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = length(increment);
	float total_distance = increment_length;

	while (total_distance < 3.6f)
	{
		u16 test_block = get_block(chunks, test_point);

		if (test_block)
		{
			set_block(chunks, test_point, BLOCK_AIR);

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
u16 place_block_raycast(Chunk_Loader* chunks, vec3 pos, vec3 dir, u16 block, uvec3* place_pos = NULL)
{
	vec3 increment = vec3(.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = length(increment);
	float total_distance = increment_length;

	while (total_distance < 4)
	{
		u16 test_block = get_block(chunks, test_point);

		if (test_block && test_block != INVALID)
		{
			set_block(chunks, test_point - increment, block);
			if (place_pos) *place_pos = uvec3(test_point - increment);
			return test_block;
		}

		total_distance += increment_length;
		test_point += increment;
	}

	return BLOCK_AIR;
}
uvec3 get_place_pos_raycast(Chunk_Loader* chunks, vec3 pos, vec3 dir)
{
	vec3 increment = vec3(.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = length(increment);
	float total_distance = increment_length;

	while (total_distance < 4)
	{
		if (get_block(chunks, test_point) && get_block(chunks, test_point) != INVALID)
		{
			return uvec3(test_point - increment); // position of where a block *would* be placed
		}

		total_distance += increment_length;
		test_point += increment;
	}

	return uvec3(INVALID);
}

void fill_sphere(Chunk_Loader* chunks, vec3 sphere_pos, float radius = 2, u16 block = BLOCK_AIR)
{
	uint chunk_x = (uint)sphere_pos.x & 0xFFF0;
	uint chunk_z = (uint)sphere_pos.z & 0xFFF0;

	vec3 local_sphere_pos = sphere_pos - vec3(chunk_x, 0, chunk_z) - vec3(.5);

	for (uint x = 0; x < CHUNK_X; ++x) {
	for (uint z = 0; z < CHUNK_Z; ++z) {
	for (uint y = 0; y < CHUNK_Y; ++y)
	{
		if (length(vec3(x, y, z) - local_sphere_pos) < radius)
			set_block(chunks, vec3(x, y, z) + vec3(chunk_x, 0, chunk_z), block);
	} } }
}
void spawn_tree(Chunk_Loader* chunks, vec3 pos)
{
	// trunk
	for (uint i = 0; i < 7; i++)
		set_block(chunks, pos + vec3(0, i, 0), BLOCK_WOOD);

	// leaves
	fill_sphere(chunks, pos + vec3(0, 6, 0), 3, BLOCK_GRASS);
}

// rendering

struct Solid_Drawable { vec3 position; float tex_offset; };
struct Fluid_Drawable { vec3 position; };

struct Chunk_Renderer
{
	uint num_solids, num_fluids;

	Solid_Drawable solids[NUM_CHUNK_BLOCKS];
	Fluid_Drawable fluids[NUM_CHUNK_BLOCKS];

	Drawable_Mesh_UV solid_mesh;
	Drawable_Mesh fluid_mesh;
};

void init(Chunk_Renderer* renderer)
{
	load(&renderer->solid_mesh, "assets/meshes/block.mesh_uv", sizeof(renderer->solids));
	mesh_add_attrib_vec3 (3, sizeof(Solid_Drawable), 0); // world pos
	mesh_add_attrib_float(4, sizeof(Solid_Drawable), sizeof(vec3)); // tex coord

	load(&renderer->fluid_mesh, "assets/meshes/fluid.mesh", sizeof(renderer->fluids));
	mesh_add_attrib_vec3(2, sizeof(Fluid_Drawable), 0); // world pos
}
void update(Chunk_Renderer* renderer, Chunk chunk, u16* blocks)
{
	Solid_Drawable* solid_mem = renderer->solids;
	Fluid_Drawable* fluid_mem = renderer->fluids;

	uint num_solids = 0, num_fluids = 0;

	for (uint x = 0; x < CHUNK_X; ++x) {
	for (uint z = 0; z < CHUNK_Z; ++z) {
	for (uint y = 0; y < CHUNK_Y; ++y)
	{
		uint index = BLOCK_INDEX(x, y, z, chunk.blocks_index);

		// if block is surrounded, don't draw it
		if (y > 0 && x > 0 && z > 0 && y < 255 && x < 15 && z < 15)
		{
			if (  blocks[index + 1]   >= BLOCK_WATER || blocks[index - 1]   >= BLOCK_WATER
				|| blocks[index + 16]  >= BLOCK_WATER || blocks[index - 16]  >= BLOCK_WATER
				|| blocks[index + 256] >= BLOCK_WATER || blocks[index - 256] >= BLOCK_WATER)
			{
				goto draw_block;
			}
		
			if (blocks[index + 1] && blocks[index - 1]
				&& blocks[index + 16] && blocks[index - 16]
				&& blocks[index + 256] && blocks[index - 256])
			{
				continue;
			}
		}
		
		draw_block:

		u16 block = blocks[index];
		if (block == BLOCK_AIR) continue; // only store block data for non-air
		
		uint block_type = 1;
		if      (block <  BLOCK_WATER) block_type = 1; // solid
		else if (block == BLOCK_WATER) block_type = 2; // fluid
		else continue;
		
		vec3 position = vec3(x, y, z) + vec3(chunk.x, 0, chunk.z);

		switch (block_type)
		{
		case 1: {
			solid_mem->position = position;
			solid_mem->tex_offset = (float)(block - 1) * (1 / 16.f);
			solid_mem++;
			num_solids++;
		} break;

		case 2: {
			fluid_mem->position = position;
			fluid_mem++;
			num_fluids++;
		} break;
		}
	} } }

	renderer->num_solids = num_solids;
	renderer->num_fluids = num_fluids;
	update(renderer->solid_mesh, num_solids * sizeof(Solid_Drawable), (byte*)renderer->solids);
	update(renderer->fluid_mesh, num_fluids * sizeof(Fluid_Drawable), (byte*)renderer->fluids);
}