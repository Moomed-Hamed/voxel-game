#include "blocks.h"

#define CHUNK_X 16
#define CHUNK_Z 16
#define CHUNK_Y 256

#define NUM_ACTIVE_CHUNKS 9 // rendered & simulated
#define NUM_BORDER_CHUNKS 9 // rendered & less simulated

#define CHUNK_BLOCK_INDEX(x,y,z) (((x) + (CHUNK_X * (z))) + ((CHUNK_X * CHUNK_Z) * (y)))
#define NUM_CHUNK_BLOCKS (CHUNK_X * CHUNK_Z * CHUNK_Y)

struct Chunk
{
	union { struct { uint32 x, z; }; uint64 id; uvec2 coords; };
	BlockID blocks[NUM_CHUNK_BLOCKS];
};

void generate_chunk_terrain(Chunk* chunk, uint offset = 256, float flatness = 32)
{
	uint x_pos = chunk->x & 0xFFF0;
	uint z_pos = chunk->z & 0xFFF0;

	BlockID* blocks = chunk->blocks;

	for (uint i = 0; i < NUM_CHUNK_BLOCKS; ++i) blocks[i] = BLOCK_AIR;

	uint8 water_level = 14;

	float n = 1.f / flatness;

	for (uint x = 0; x < CHUNK_X; ++x) {
	for (uint z = 0; z < CHUNK_Z; ++z) {

		float noise_value = perlin(n * (x + x_pos + offset), n * (z + z_pos + offset));
		uint height = 4 + (noise_value * 32);
		
		for (uint y = 0; y < CHUNK_Y; ++y)
		{
			uint index = CHUNK_BLOCK_INDEX(x, y, z);
			blocks[index] = BLOCK_AIR;
		
			if (height <= water_level)
			{
				if (y > water_level)
					break;
				else if (y == water_level)
					blocks[index] = BLOCK_WATER;
				else if (y < 16)
				{
					if (__rdtsc() % 256 == 0)
						blocks[index] = BLOCK_ORES;
					else
						blocks[index] = BLOCK_STONE;
				}
				else if (y <= height)
					blocks[index] = BLOCK_SAND;
				else
					blocks[index] = BLOCK_AIR;
			}
			else // land
			{
				if (y > height)
					break;
				else if (height == water_level + 1 && y > 15)
					blocks[index] = BLOCK_SAND;
				else if (y == height)
				{
					blocks[index] = BLOCK_GRASS;
					if (__rdtsc() % 1250 == 0)
						int a = 0;
					//chunk_place_tree(chunk.blocks, x, y, z);
				}
				else if (y == height - 1 || y == height - 2 || y == height - 3)
					blocks[index] = BLOCK_DIRT;
				else
				{
					if (__rdtsc() % 256 == 0)
						blocks[index] = BLOCK_ORES;
					else
						blocks[index] = BLOCK_STONE;
				}
			}
		
			if (height == water_level + 1) blocks[index] = BLOCK_SAND;
		
		}
	} } //blocks[BLOCK_INDEX(8, 64, 8)] = CLAY_BRICK_ID;
}

// rendering

struct Chunk_Renderer
{
	uint num_solids, num_fluids;

	Solid_Drawable solids[NUM_CHUNK_BLOCKS];
	Fluid_Drawable fluids[NUM_CHUNK_BLOCKS];

	Drawable_Mesh_UV solid_mesh;
	Drawable_Mesh fluid_mesh;
	Shader solid_shader, fluid_shader;
};

void init(Chunk_Renderer* renderer)
{
	load(&renderer->solid_mesh, "assets/meshes/block.mesh_uv", "assets/textures/block_atlas.bmp", sizeof(renderer->solids));
	mesh_add_attrib_vec3(3, sizeof(Solid_Drawable), 0); // world pos
	mesh_add_attrib_vec2(4, sizeof(Solid_Drawable), sizeof(vec3)); // tex coord

	load(&(renderer->solid_shader), "assets/shaders/solid.vert", "assets/shaders/mesh_uv.frag");
	bind(renderer->solid_shader);
	set_int(renderer->solid_shader, "positions", 0);
	set_int(renderer->solid_shader, "normals"  , 1);
	set_int(renderer->solid_shader, "albedo"   , 2);
	set_int(renderer->solid_shader, "texture_sampler", 3);

	load(&renderer->fluid_mesh, "assets/meshes/fluid.mesh", sizeof(renderer->fluids));
	mesh_add_attrib_vec3(2, sizeof(Fluid_Drawable), 0); // world pos

	load(&(renderer->fluid_shader), "assets/shaders/fluid.vert", "assets/shaders/mesh.frag");
	bind(renderer->fluid_shader);
	set_int  (renderer->fluid_shader, "positions", 0);
	set_int  (renderer->fluid_shader, "normals"  , 1);
	set_int  (renderer->fluid_shader, "albedo"   , 2);
	set_float(renderer->fluid_shader, "timer"    , 1.f);
}
void update_renderer(Chunk_Renderer* renderer, Chunk* chunk)
{
	uint num_solids = 0;
	uint num_fluids = 0;

	Solid_Drawable* solid_mem = renderer->solids;
	Fluid_Drawable* fluid_mem = renderer->fluids;

	const BlockID* blocks = chunk->blocks;
	uint chunk_x = chunk->x;
	uint chunk_z = chunk->z;

	for (uint x = 0; x < CHUNK_X; ++x) {
	for (uint z = 0; z < CHUNK_Z; ++z) {
	for (uint y = 0; y < CHUNK_Y; ++y)
	{
		uint index = CHUNK_BLOCK_INDEX(x, y, z);

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

		BlockID block = blocks[index];
		if (block == BLOCK_AIR) continue; // only store block data for non-air

		uint block_type = 1;
		if (block < BLOCK_WATER) block_type = 1; // solid
		else if (block < BLOCK_CROP) block_type = 2; // fluid
		else continue;

		vec3 position = vec3(chunk_x + x, y, chunk_z + z);

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