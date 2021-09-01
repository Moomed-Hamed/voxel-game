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

// terrain manipulation
void set_block(Chunk* chunks, vec3 pos, BlockID new_block)
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
void set_block(Chunk* chunks, uvec3 coords, BlockID new_block)
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
BlockID get_block(Chunk* chunks, vec3 pos)
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
BlockID get_block_raycast(Chunk* chunks, vec3 pos, vec3 dir)
{
	vec3 increment = vec3(0.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 3.6f)
	{
		BlockID block = get_block(chunks, test_point);
		if (block) return block;

		total_distance += increment_length;
		test_point += increment;
	}

	return INVALID;
}
BlockID break_block_raycast(Chunk* chunks, vec3 pos, vec3 dir, vec3* breakpos = NULL)
{
	vec3 increment = vec3(0.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 3.6f)
	{
		BlockID test_block = get_block(chunks, test_point);

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
BlockID place_block_raycast(Chunk* chunks, vec3 pos, vec3 dir, BlockID block, uvec3* place_pos = NULL)
{
	vec3 increment = vec3(0.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = glm::length(increment);
	float total_distance = increment_length;

	while (total_distance < 4)
	{
		BlockID test_block = get_block(chunks, test_point);

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
void get_place_pos_raycast(Chunk* chunks, vec3 pos, vec3 dir, uvec3* place_pos)
{
	vec3 increment = vec3(0.2) * normalize(dir);
	vec3 test_point = pos + increment;

	float increment_length = length(increment);
	float total_distance = increment_length;

	while (total_distance < 4)
	{
		if (get_block(chunks, test_point) && get_block(chunks, test_point) != INVALID)
		{
			*place_pos = uvec3(test_point - increment); // position of where a block *would* be placed
			return;
		}

		total_distance += increment_length;
		test_point += increment;
	}

	place_pos->x = INVALID;
}
bool collide(Chunk* chunks, Cube_Collider_AA* object)
{
	vec3 position = object->position;
	vec3 block_pos = vec3(floor(position.x), floor(position.y), floor(position.z));

	Cube_Collider_AA collider = {};
	collider.position = block_pos;
	collider.scale = vec3(1);

	// object must be smaller than 1x1x1 for this to work
	if (cube_aa_cube_aa_intersect(*object, collider))
	{
		object->force.y = position.y - block_pos.y;

		//float y_force = position.x

		//object->force = position - (block_pos + vec3(.5, 0, .5));
		//printvec(object->force);
		return true;
	}
	return false;
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
	load(&renderer->solid_mesh, "assets/meshes/block.mesh_uv", sizeof(renderer->solids));
	mesh_add_attrib_vec3 (3, sizeof(Solid_Drawable), 0); // world pos
	mesh_add_attrib_float(4, sizeof(Solid_Drawable), sizeof(vec3)); // tex coord

	GLuint texture_id  = load_texture("assets/textures/block_atlas.bmp");
	GLuint material_id = load_texture("assets/textures/atlas_mat.bmp");

	renderer->solid_mesh.texture_id  = texture_id;
	renderer->solid_mesh.material_id = material_id;
	load(&(renderer->solid_shader), "assets/shaders/solid.vert", "assets/shaders/mesh_uv.frag");

	load(&renderer->fluid_mesh, "assets/meshes/fluid.mesh", sizeof(renderer->fluids));
	mesh_add_attrib_vec3(2, sizeof(Fluid_Drawable), 0); // world pos

	load(&(renderer->fluid_shader), "assets/shaders/fluid.vert", "assets/shaders/mesh.frag");
	bind(renderer->fluid_shader);
	set_float(renderer->fluid_shader, "timer" , 1.f);
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
void draw(Chunk_Renderer* renderer, mat4 proj_view, float timer)
{
	bind(renderer->solid_shader);
	set_mat4(renderer->solid_shader, "proj_view", proj_view);
	bind_texture(renderer->solid_mesh);
	
	draw(renderer->solid_mesh, renderer->num_solids);
	
	bind(renderer->fluid_shader);
	set_mat4(renderer->fluid_shader, "proj_view", proj_view);
	set_float(renderer->fluid_shader, "timer", timer);
	
	draw(renderer->fluid_mesh, renderer->num_fluids);
}