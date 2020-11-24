#include "automata.h"

#define PLAYER_BACKPACK_SIZE 27
#define PLAYER_HOTBAR_SIZE   9
#define PLAYER_INVENTORY_SIZE (PLAYER_BACKPACK_SIZE + PLAYER_HOTBAR_SIZE)

struct Player_Inventory
{
	uint selected_item_index;

	union // makes handling the inventory easier
	{
		Item items[PLAYER_INVENTORY_SIZE];

		struct {
			Item backpack[PLAYER_BACKPACK_SIZE];
			Item hotbar[PLAYER_HOTBAR_SIZE];
		};
	};
};

enum PLAYER_ACTION {
	MINING, DRINKING
};

struct Player_One
{
	vec3 position;
	vec3 velocity;

	uint current_action;
	float action_time;

	uint mining_level; // mining speed
	uint damage_level; // attack damage
	
	Player_Inventory inventory;

	vec3 front, right, up;
	float yaw, pitch;
};

void ready_player_one(Player_One* player, vec3 pos = { 20, 24, 20 })
{
	*player = { pos }; // {} = zero the other fields
	player->action_time = -10;
}

// returns the count of items stored
uint player_store_item(Player_One* player, Item item)
{
	for (uint i = 0; i < PLAYER_INVENTORY_SIZE; i++)
	{
		if (player->inventory.items[i].main_type == NONE)
		{
			player->inventory.items[i] = item;
			return item.count;
		}
		else if (player->inventory.items[i].main_type == item.main_type)
		{
			player->inventory.items[i].count++;
			return item.count;
		}
	}
}
void player_pickup_loose_items(Player_One* player, Loose_Item* items)
{
	for (uint i = 0; i < MAX_LOOSE_ITEMS; i++)
	{
		if (items[i].main_type == NONE) continue;

		float distance = glm::length(player->position - items[i].position);

		if (distance < 2.1)
		{
			player_store_item(player, items[i]);
			items[i] = {};
		}
	}
}

void player_set_action(Player_One* player, uint action)
{
	player->current_action = action;

	switch (action)
	{
	case PLAYER_ACTION::MINING:
	{
		// switch(player->tool)
		player->action_time = 1;
	}break;
	}
}

void player_break_block(Player_One* player, Chunk* chunks)
{
	switch (player->current_action)
	{
		case PLAYER_ACTION::MINING:
		{
			if (player->action_time > 0)
			{
				player->action_time -= .1;
			}
			else
			{
				world_break_block_raycast(chunks, player->position, player->front);
				player_set_action(player, NONE);
			}
		} return;
	} //switch (player->inventory.hotbar[player->inventory.selected_item_index].main_type)
}
void player_place_block(Player_One* player, Chunk* chunks)
{
	if (player->inventory.hotbar[player->inventory.selected_item_index].main_type != BLOCK_ITEM) return;

	BlockID block = BLOCK::STONE_BRICK;
	world_place_block_raycast(chunks, player->position, player->front, block);
}

void player_update_dir(Player_One* player, float x_offset, float y_offset)
{
	player->yaw   += (x_offset * .003f) / TWOPI;
	player->pitch += (y_offset * .003f) / TWOPI;

	if (player->pitch >  PI / 2.01) player->pitch =  PI / 2.01;
	if (player->pitch < -PI / 2.01) player->pitch = -PI / 2.01;

	player->front.y = sin(player->pitch);
	player->front.x = cos(player->pitch) * cos(player->yaw);
	player->front.z = cos(player->pitch) * sin(player->yaw);

	player->front = glm::normalize(player->front);
	player->right = glm::normalize(glm::cross(player->front, vec3(0, 1, 0)));
	player->up    = glm::normalize(glm::cross(player->right, player->front));
}

void player_move(Player_One* player, int dir, float distance, bool sprint = false)
{
	if (sprint) distance *= 1.5;
	if (dir == DIR_FORWARD ) player->position += (player->front * vec3(1,0,1)) * distance;
	if (dir == DIR_LEFT    ) player->position -= (player->right * vec3(1,0,1)) * distance;
	if (dir == DIR_RIGHT   ) player->position += (player->right * vec3(1,0,1)) * distance;
	if (dir == DIR_BACKWARD) player->position -= (player->front * vec3(1,0,1)) * distance;

	if (player->position.x < 0) player->position.x = 0;
	if (player->position.y < 0) player->position.y = 0;
	if (player->position.z < 0) player->position.z = 0;
}
void player_update_pos(Player_One* player, Chunk* chunks, float dtime)
{
	vec3 pos = player->position;
	vec3 vel = player->velocity;

	if (pos.y < 1) pos.y = 1;

	pos += player->velocity * dtime;

	if (world_get_block(chunks, pos - vec3(0, 2, 0))) // player is 2 blocks tall
	{
		float new_pos = ceil(pos.y);
		float bounce_force = (new_pos - pos.y) * 2;

		pos.y = new_pos;
		vel.y = bounce_force;
	}
	else vel.y += GRAVITY;

	player->position = pos;
	player->velocity = vel;
}