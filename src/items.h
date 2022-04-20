#include "chunk.h"

#define ITEM_BLOCK		1 // placeable block
#define ITEM_TOOL			2 // pick, sword, bow, etc.
#define ITEM_RESOURCE	3 // coal, sticks, etc
#define ITEM_INGOT		4 // iron, copper, gold, etc
#define ITEM_PIPE			5 // wood, stone, iron, etc.
#define ITEM_WIRE			6 // copper, insulated, optic, etc.

#define TOOL_PICKAXE	1
#define TOOL_SHOVEL	2
#define TOOL_AXE		3
#define TOOL_SWORD	4
#define TOOL_BOW		5

#define INGOT_IRON	1
#define INGOT_COPPER	2
#define INGOT_GOLD	3
#define INGOT_SILVER	4
#define INGOT_STEEL	5

#define RESOURCE_COAL		1
#define RESOURCE_CHARCOAL	2
#define RESOURCE_DIAMOND	3
#define RESOURCE_CRUSHED_IRON_ORE 4
#define RESOURCE_WASHED_IRON_ORE 5
#define RESOURCE_IRON_NUGGET 6

#define PIPE_STONE	1
#define PIPE_IRON		2
#define ITEM_GOLD		3
#define ITEM_COPPER	4
#define PIPE_DIAMOND	5

struct Item { uint type, id, count; };

Item smelt(Item item)
{
	switch (item.id)
	{
	case BLOCK_WOOD : {
		return Item{ ITEM_RESOURCE, RESOURCE_CHARCOAL, 1 };
	} break;
	}

	return Item{}; // cannot smelt item
}
Item crush(Item item)
{
	switch (item.id)
	{
	case BLOCK_IRON_ORE: {
		return Item{ ITEM_RESOURCE, RESOURCE_CRUSHED_IRON_ORE, 1 };
	} break;
	}

	return Item{}; // cannot smelt item
}
Item wash(Item item)
{
	switch (item.id)
	{
	case RESOURCE_CRUSHED_IRON_ORE: {
		return Item{ ITEM_RESOURCE, RESOURCE_WASHED_IRON_ORE, 1 };
	} break;
	}

	return Item{}; // cannot smelt item
}

struct Furnace
{
	Item in, out, fuel;
	float progress;
	float fuel_level;
};

void update(Furnace* furnace, float dt)
{
	if (furnace->in.type)
	{
		furnace->fuel_level -= dt;
		furnace->out = smelt(furnace->in);
		furnace->in.count--;
	}
}

struct Crafting_Table
{
	Item in[9];
	Item out;
};

#define MAX_CHESTS 1
#define NUM_CHEST_ITEMS (12 * 4)

struct Chest
{
	Item items[NUM_CHEST_ITEMS];
	uvec3 coord;
};

Item get_next_item(Chest* chests, uvec3 coords)
{
	Chest* chest = {};

	for (uint i = 0; i < MAX_CHESTS; i++)
	{
		if (chests[i].coord == coords)
		{
			chest = chests + i;
			break;
		}
	}

	if (chest)
	{
		for (uint i = 0; i < NUM_CHEST_ITEMS; i++)
		{
			if (chest->items[i].type == NULL) continue;

			Item ret = chest->items[i];

			if (chest->items[i].count > 1)
			{
				ret.count = 1;
				return ret;
			}
			else
			{
				chest->items[i] = {};
				return ret;
			}
		}
	}

	return Item{}; // chest is empty
}
void store_item(Chest* chests, Item item) {}

// making these should be an interesting logic puzzle

// general purpose(?)
#define UP		0x1
#define DOWN	0x2
#define LEFT	0x4
#define RIGHT	0x8
#define FRONT	0x16
#define BACK	0x32

#define MAX_PIPES 1

struct Pipe
{
	uint type;
	uint inputs;
	uvec3 coords;

	float progress;
	Item item;
	uint dir;
};

void update(Pipe* pipes, float dt)
{
	for (uint i = 0; i < MAX_PIPES; i++)
	{
		if (pipes[i].type == NULL) continue;

		if (pipes[i].item.type) // pipe is transporting
		{
			pipes[i].progress += dt;

			if (pipes[i].progress > 1) // deposit item
			{
				// store_item(chests, [depends on dir])
			}
		}
		else if (pipes[i].inputs & DOWN) // pipe is empty
		{
			pipes[i].item = {};// get_next_item(chests, coords - uvec3(0, 1, 0));
		}
	}
}