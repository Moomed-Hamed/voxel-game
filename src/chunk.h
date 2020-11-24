#include "renderer.h"

enum BLOCK {
	AIR = 0,
	STONE, DIRT, GRASS, SAND,
	WOOD, LEAF,
	COAL_ORE, IRON_ORE,
	WOOD_PLANK, CLAY_BRICK, STONE_BRICK,

	MAX_SOLID,

	WATER, LAVA,

	MAX_FLUID,

	CROP
};

#define CHUNK_RENDER_DISTANCE 4
#define NUM_ACTIVE_CHUNKS (CHUNK_RENDER_DISTANCE * CHUNK_RENDER_DISTANCE)

#define CHUNK_X 16
#define CHUNK_Z 16
#define CHUNK_Y 256

#define CHUNK_BLOCK_INDEX(x,y,z) (((x) + (CHUNK_X * (z))) + ((CHUNK_X * CHUNK_Z) * (y)))
#define NUM_CHUNK_BLOCKS (CHUNK_X * CHUNK_Z * CHUNK_Y)

typedef uint32 BlockID;

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

	for (uint i = 0; i < NUM_CHUNK_BLOCKS; ++i) blocks[i] = BLOCK::AIR;

	uint8 water_level = 14;

	float n = 1.f / flatness;

	for (uint x = 0; x < CHUNK_X; ++x) {
	for (uint z = 0; z < CHUNK_Z; ++z) {

		float noise_value = perlin(n * (x + x_pos + offset), n * (z + z_pos + offset));
		uint height = 4 + (noise_value * 32);
		
		for (uint y = 0; y < CHUNK_Y; ++y)
		{
			uint index = CHUNK_BLOCK_INDEX(x, y, z);
			blocks[index] = BLOCK::AIR;
		
			if (height <= water_level)
			{
				if (y > water_level)
					break;
				else if (y == water_level)
					blocks[index] = BLOCK::WATER;
				else if (y < 16)
				{
					if (__rdtsc() % 256 == 0)
						blocks[index] = BLOCK::COAL_ORE;
					else
						blocks[index] = BLOCK::STONE;
				}
				else if (y <= height)
					blocks[index] = BLOCK::SAND;
				else
					blocks[index] = BLOCK::AIR;
			}
			else // land
			{
				if (y > height)
					break;
				else if (height == water_level + 1 && y > 15)
					blocks[index] = BLOCK::SAND;
				else if (y == height)
				{
					blocks[index] = BLOCK::GRASS;
					if (__rdtsc() % 1250 == 0)
						int a = 0;
					//chunk_place_tree(chunk.blocks, x, y, z);
				}
				else if (y == height - 1 || y == height - 2 || y == height - 3)
					blocks[index] = BLOCK::DIRT;
				else
				{
					if (__rdtsc() % 256 == 0)
						blocks[index] = BLOCK::COAL_ORE;
					else
						blocks[index] = BLOCK::STONE;
				}
			}
		
			if (height == water_level + 1) blocks[index] = BLOCK::SAND;
		
		}
	} } //blocks[BLOCK_INDEX(8, 64, 8)] = CLAY_BRICK_ID;
}

// -- rendering -- //

vec2 block_get_tex_offset(BlockID block)
{
	switch (block)
	{
	case BLOCK::STONE:       return vec2(0.0, 0.00);
	case BLOCK::DIRT:        return vec2(.25, 0.00);
	case BLOCK::GRASS:       return vec2(.50, 0.00);
	case BLOCK::SAND:        return vec2(.75, 0.00);
	case BLOCK::WOOD:        return vec2(0.0, -.25);
	case BLOCK::WOOD_PLANK:  return vec2(.25, -.25);
	case BLOCK::CLAY_BRICK:  return vec2(.50, -.25);
	case BLOCK::STONE_BRICK: return vec2(.75, -.25);
	case BLOCK::COAL_ORE:    return vec2(0.0, -.50);
	//case 10: return vec2(.25, -.50);
	//case 11: return vec2(.50, -.50);
	//case 12: return vec2(.75, -.50);
	//case 13: return vec2(0.0, -.75);
	//case 14: return vec2(.25, -.75);
	//case 15: return vec2(.50, -.75);
	default: return vec2(.75, -.75); // invalid block
	}
}

struct Solid_Drawable { vec3 position; vec2 tex_offset; };
struct Fluid_Drawable { vec3 position; };

struct Chunk_Renderer
{
	uint num_solids, num_fluids;

	Solid_Drawable solids[NUM_CHUNK_BLOCKS * NUM_ACTIVE_CHUNKS];
	Fluid_Drawable fluids[NUM_CHUNK_BLOCKS * NUM_ACTIVE_CHUNKS];

	Drawable_Mesh_UV solid_mesh;
	Drawable_Mesh fluid_mesh;
	Shader solid_shader, fluid_shader;
};

void init(Chunk_Renderer* renderer)
{
	load(&renderer->solid_mesh, "assets/meshes/block.model", "assets/textures/texture_atlas.bmp", sizeof(renderer->solids));
	mesh_add_attrib_vec3(3, sizeof(Solid_Drawable), 0); // world pos
	mesh_add_attrib_vec2(4, sizeof(Solid_Drawable), sizeof(vec3)); // tex coord

	load(&renderer->fluid_mesh, "assets/meshes/fluid.model", sizeof(renderer->fluids));
	mesh_add_attrib_vec3(2, sizeof(Fluid_Drawable), 0); // world pos

	load(&(renderer->solid_shader), "assets/shaders/solid.vert", "assets/shaders/solid.frag");
	bind(renderer->solid_shader);
	set_int(renderer->solid_shader, "positions", 0);
	set_int(renderer->solid_shader, "normals", 1);
	set_int(renderer->solid_shader, "albedo", 2);
	set_int(renderer->solid_shader, "texture_sampler", 3);

	load(&(renderer->fluid_shader), "assets/shaders/fluid.vert", "assets/shaders/fluid.frag");
	bind(renderer->fluid_shader);
	set_int  (renderer->fluid_shader, "positions", 0);
	set_int  (renderer->fluid_shader, "normals"  , 1);
	set_int  (renderer->fluid_shader, "albedo"   , 2);
	set_float(renderer->fluid_shader, "timer"    , 1.f);
}
void update_renderer(Chunk_Renderer* renderer, Chunk* chunks)
{
	uint num_solids = 0;
	uint num_fluids = 0;

	Solid_Drawable* solid_mem = renderer->solids;
	Fluid_Drawable* fluid_mem = renderer->fluids;

	for (int i = 0; i < NUM_ACTIVE_CHUNKS; i++)
	{
		const BlockID* blocks = chunks[i].blocks;
		uint chunk_x = chunks[i].x;
		uint chunk_z = chunks[i].z;

		for (uint x = 0; x < CHUNK_X; ++x) {
		for (uint z = 0; z < CHUNK_Z; ++z) {
		for (uint y = 0; y < CHUNK_Y; ++y)
		{
			uint index = CHUNK_BLOCK_INDEX(x, y, z);

			// if block is surrounded, don't draw it
			if (y > 0 && x > 0 && z > 0 && y < 255 && x < 15 && z < 15)
			{
				if (  blocks[index + 1]   >= BLOCK::MAX_SOLID || blocks[index - 1]   >= BLOCK::MAX_SOLID
					|| blocks[index + 16]  >= BLOCK::MAX_SOLID || blocks[index - 16]  >= BLOCK::MAX_SOLID
					|| blocks[index + 256] >= BLOCK::MAX_SOLID || blocks[index - 256] >= BLOCK::MAX_SOLID)
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
			if (block == BLOCK::AIR) continue; // only store block data for non-air

			uint block_type = 1;
			if (block < BLOCK::MAX_SOLID) block_type = 1; // solid
			else if (block < BLOCK::MAX_FLUID) block_type = 2; // fluid
			else continue;

			vec3 position = vec3(chunk_x + x, y, chunk_z + z);

			switch (block_type)
			{
			case 1: {
				solid_mem->position = position;
				solid_mem->tex_offset = block_get_tex_offset(block);
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
	}

	renderer->num_solids = num_solids;
	renderer->num_fluids = num_fluids;
	update(renderer->solid_mesh, num_solids * sizeof(Solid_Drawable), (byte*)renderer->solids);
	update(renderer->fluid_mesh, num_fluids * sizeof(Fluid_Drawable), (byte*)renderer->fluids);
}