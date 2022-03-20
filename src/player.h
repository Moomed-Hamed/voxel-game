#include "gui.h"

#define STATUS_NEUTRAL	0
#define STATUS_IN_MENU	1

#define ACTION_MINING 1
#define ACTION_ATTACK 2
#define ACTION_ITEM_SELECTED 3
#define ACTION_INTERACT 4

struct Player
{
	Camera eyes;
	union { struct { Item hotbar[NUM_HOTBAR_ITEMS]; Item inventory[NUM_INVENTORY_ITEMS]; }; Item items[NUM_PLAYER_ITEMS]; };
	struct { float attack, mine; } power;

	uint status, action;
	float action_progress, action_time;
	
	uint equipped_item; // for neutral (which hotbar item is selected)
	
	// UI
	int selected_item;
	Item crafting[10]; // 9 in + 1 out
	Item* opened_items; // for opening containiers
};

void init(Player* player)
{
	player->eyes = { {116, 48, 116} };
	player->inventory[0]  = Item{ ITEM_BLOCK, BLOCK_STONE, 1 };
	player->inventory[16] = Item{ ITEM_BLOCK, BLOCK_FURNACE, 1 };
}
void update(Player* player, World* world, Keyboard keys, Mouse mouse, float dt, Particle_Emitter* emitter, Audio* pops, Icon_Drawable* icons)
{
	switch (player->status)
	{
	case STATUS_NEUTRAL: goto neutral;
	case STATUS_IN_MENU: goto in_menu;
	}

neutral:
	if (FirstPress(keys.I))
	{
		player->status = STATUS_IN_MENU;
		player->action = GUI_INVENTORY;
		player->opened_items = player->crafting;
		player->selected_item = -1;
		player->action_time = player->action_progress = 0;
		return;
	}

	// movement
	update_dir(&player->eyes, mouse.dx, mouse.dy, dt);
	if (keys.W.is_pressed) update_pos(&player->eyes, DIR_FORWARD , dt * 10.f);
	if (keys.S.is_pressed) update_pos(&player->eyes, DIR_BACKWARD, dt * 10.f);
	if (keys.A.is_pressed) update_pos(&player->eyes, DIR_LEFT    , dt * 10.f);
	if (keys.D.is_pressed) update_pos(&player->eyes, DIR_RIGHT   , dt * 10.f);

	if (FirstPress(mouse.left_button))
	{
		u16 block = get_block_raycast(&world->chunks, player->eyes.position, player->eyes.front);

		if (block == BLOCK_WATER)
			return;

		switch (player->hotbar[player->equipped_item].id)
		{
		case TOOL_SWORD: out("attack"); break;
		default: { // break block
			vec3 break_pos = {};
			block = break_block_raycast(&world->chunks, player->eyes.position, player->eyes.front, &break_pos);
			spawn(world->items, Item{ ITEM_BLOCK, block, 1 }, break_pos);
			emit_blockbreak(emitter, break_pos);
			play_audio(pops[3]);
		}
		}
	}

	if (FirstPress(mouse.right_button))
	{
		u16 block = get_block_raycast(&world->chunks, player->eyes.position, player->eyes.front);
		if (block == INVALID) return;

		switch (block)
		{
		case BLOCK_FURNACE: out("opening inventory"); break;
		default: { // place block
			place_block_raycast(&world->chunks, player->eyes.position, player->eyes.front, player->hotbar[player->equipped_item].id);
		}
		}
	}

	return;

in_menu:
	if (FirstPress(keys.I) || FirstPress(keys.ESC))
	{
		player->status = STATUS_NEUTRAL;
		player->action = NULL;
		player->equipped_item = 0;
		player->action_time = player->action_progress = 0;
		return;
	}

	int selected_index = player->selected_item;
	int hovered_index  = gui_item_index(icons, mouse, selected_index);

	if (hovered_index >= NUM_PLAYER_ITEMS) out("not on player");

	if (selected_index < 0) // nothing selected
	{
		if (FirstPress(mouse.left_button))
		{
			if (hovered_index < 0 || player->items[hovered_index].type == NULL)
				return;
			else
				player->selected_item = hovered_index;
		}
	}
	else  // dragging an item
	{
		if (FirstPress(mouse.left_button))
		{
			if (hovered_index < 0) // add to world items?
				return;
			else if (hovered_index == selected_index) // put it back
				player->selected_item = -1;
			else // TODO : Handle items that are not in player items
			{
				if (player->items[hovered_index].type) // swap
				{
					Item temp = player->items[selected_index];
					player->items[selected_index] = player->items[hovered_index];
					player->items[hovered_index] = temp;
				}
				else // deposit
				{
					player->selected_item = -1;
					player->items[hovered_index] = player->items[selected_index];
					player->items[selected_index] = {};
				}
			}
		}
	}

	return;
}

uint give_player_item(Item* player_items, Item item)
{
	for (uint i = 0; i < NUM_PLAYER_ITEMS; i++)
	{
		if (player_items[i].type == NULL)
		{
			player_items[i] = item;
			return 1; // should this be an index or something
		}
	}

	return 0; // no slot found
}