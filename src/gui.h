#include "world.h"

#define NUM_HOTBAR_ITEMS 12
#define NUM_INVENTORY_ITEMS (NUM_HOTBAR_ITEMS * 4)
#define NUM_PLAYER_ITEMS (NUM_HOTBAR_ITEMS + NUM_INVENTORY_ITEMS)

#define MAX_GUI_QUADS 128

vec2 rotate(float angle, vec2 v) // not sure if this is correct
{
	glm::mat2 r = glm::mat2{ cos(angle), sin(angle), -1.f * sin(angle), cos(angle) };
	return r * v;
}

struct Quad_Drawable
{
	vec2 position;
	vec2 scale;
	vec3 color;
};

struct Icon_Drawable
{
	vec2 position;
	vec2 scale;
	vec2 tex_offset;
};

bool mouse_in_quad(Quad_Drawable quad, Mouse mouse)
{
	if (mouse.norm_x < quad.position.x - quad.scale.x) return false;
	if (mouse.norm_y < quad.position.y - quad.scale.y) return false;
	if (mouse.norm_x > quad.position.x + quad.scale.x) return false;
	if (mouse.norm_y > quad.position.y + quad.scale.y) return false;
	return true;
}
bool mouse_in_quad(Icon_Drawable quad, Mouse mouse)
{
	if (mouse.norm_x < quad.position.x - quad.scale.x) return false;
	if (mouse.norm_y < quad.position.y - quad.scale.y) return false;
	if (mouse.norm_x > quad.position.x + quad.scale.x) return false;
	if (mouse.norm_y > quad.position.y + quad.scale.y) return false;
	return true;
}
int gui_item_index(Icon_Drawable* icons, Mouse mouse, int selected_index = -1)
{
	for (int i = 0; i < NUM_PLAYER_ITEMS; i++)
	{
		// icons are in the order : [hotbar][backpack][interactable(chest, furnace, etc.)]
		if (mouse_in_quad(icons[i], mouse) && i != selected_index)
			return i;
	}

	return -1;
}

#define GUI_INVENTORY	1
#define GUI_FURNACE		2
#define GUI_CRUSHER		3
#define GUI_WASHER		4
#define GUI_CRAFTING		5
#define GUI_RECYCLER		6

struct GUI_Renderer
{
	Quad_Drawable quads[MAX_GUI_QUADS];
	Icon_Drawable icons[MAX_GUI_QUADS];
	Drawable_Mesh_2D quad_mesh;
	Drawable_Mesh_2D_UV icon_mesh;
	Shader quad_shader, icon_shader;
	GLuint texture;
};

void init(GUI_Renderer* renderer)
{
	init(&renderer->quad_mesh, MAX_GUI_QUADS * sizeof(Quad_Drawable));
	mesh_add_attrib_vec2(1, sizeof(Quad_Drawable), 0 * sizeof(vec2)); // position
	mesh_add_attrib_vec2(2, sizeof(Quad_Drawable), 1 * sizeof(vec2)); // scale
	mesh_add_attrib_vec3(3, sizeof(Quad_Drawable), 2 * sizeof(vec2)); // color

	init(&renderer->icon_mesh, MAX_GUI_QUADS * sizeof(Icon_Drawable), {}, vec2(1.f / 16));
	mesh_add_attrib_vec2(2, sizeof(Icon_Drawable), 0 * sizeof(vec2)); // position
	mesh_add_attrib_vec2(3, sizeof(Icon_Drawable), 1 * sizeof(vec2)); // scale
	mesh_add_attrib_vec2(4, sizeof(Icon_Drawable), 2 * sizeof(vec2)); // texture offset

	renderer->texture = load_texture("assets/textures/icons.bmp", false);
	load(&renderer->quad_shader, "assets/shaders/mesh_2D.vert"   , "assets/shaders/mesh_2D.frag"   );
	load(&renderer->icon_shader, "assets/shaders/mesh_2D_UV.vert", "assets/shaders/mesh_2D_UV.frag");
}
void update(GUI_Renderer* renderer, Mouse mouse, Item* player_items, uint screen = 1, int selected_index = -1, Item* items = NULL)
{
	// screen = which GUI should be displayed : inventory, crafting table, furnace, etc.
	// selected_index : item the player has equipped : -1 = none, 3 = 4th inventory slot, 
	//                                                 > NUM_PLAYER_INVENTORY_ITEMS = item is not on the player
	// items = items for whatever the player is interacting with(eg. chest); NULL = nothing open
	// player_items : player inventory

	// Warning : this system is a first draft, it relies on alot of obscure details
	// The order in which icons are added to the render buffer matters, see get_item_index for more information

	ZeroMemory(renderer->quads, sizeof(renderer->quads));
	ZeroMemory(renderer->icons, sizeof(renderer->icons));

	uint num_quads = 0, num_icons = 0;

	vec2 scale = vec2(.3, .5); // makes a square in a 16:9 screen (i think)
	vec3 color = vec3(.1);

	// texture coordinates of an item
	auto tex = [](u16 block) {u16 m = block % 16; return vec2(m / 16.f, (block - m) / 16.f); };

	// --- icons --- //
	
	// hotbar
	for (uint i = 0; i < NUM_HOTBAR_ITEMS; i++)
		renderer->icons[num_icons++] = { vec2(-.66 + (i * .12), -.799), scale / 6.f, tex(player_items[i].id) };

	if (screen) // inventory
		for (uint i = 0, n = 0; i < 12; i++)
		for (uint j = 3; j < 6; j++)
			renderer->icons[num_icons++] = { vec2(-.66 + (i * .12), .6 - (j * .2)), scale / 6.f, tex(player_items[NUM_HOTBAR_ITEMS + n++].id) };

	// --- quads --- //
	
	// hotbar selected item
	renderer->quads[num_quads++] = { vec2(-.66 + (0 * .12), -.799), scale / 5.6f, vec3(.3) };

	// hotbar item frames
	for (uint i = 0; i < 12; i++)
		renderer->quads[num_quads++] = { vec2(-.66 + (i * .12), -.799), scale / 6.f, vec3(.2) };

	// hotbar frame
	renderer->quads[num_quads++] = { vec2(0, -.8), vec2(.735, .12), color };

	switch (screen)
	{
	case GUI_INVENTORY:
	{
		// icons
		for (uint i = 0, n = 0; i < 3; i++)
		for (uint j = 0; j < 3; j++)
			renderer->icons[num_icons++] = { vec2(-.66 + (i * .12), .7 - (j * .2)), scale / 6.f, tex(items[n++].id) };
		
		// crafting window
		for (uint i = 0; i < 3; i++)
		for (uint j = 0; j < 3; j++)
			renderer->quads[num_quads++] = { vec2(-.66 + (i * .12), .7 - (j * .2)), scale / 6.f, vec3(.4) };
		
		// crafting output
		renderer->quads[num_quads++] = { vec2(-.66 + (4 * .12), .7 - (1 * .2)), scale / 6.f, vec3(.4) };
	} break;
	case GUI_CRAFTING:
	{
		for (uint i = 0; i < 12; i++)
		for (uint j = 0; j <  6; j++)
			renderer->quads[num_quads++] = { vec2(-.66 + (i * .12), .6 - (j * .2)), scale / 6.f, vec3(.2) };

		renderer->quads[num_quads++] = { vec2(0, 0), vec2(.73, .85), color };
	} break;
	}

	if (screen) // player backpack items + background
	{
		// player item backgrounds
		for (uint i = 0; i < 12; i++)
		for (uint j = 3; j <  6; j++)
			renderer->quads[num_quads++] = { vec2(-.66 + (i * .12), .6 - (j * .2)), scale / 6.f, vec3(.2) };
	
		// background
		renderer->quads[num_quads++] = { vec2(0, 0), vec2(.735, .86), color };
	}
	else
	{
		for (uint i = 0; i < 10; i++) // health
			renderer->quads[num_quads++] = { vec2(-.7 + (i * .06), -.63), scale / 12.f, vec3(.4, 0, 0) };

		for (uint i = 0; i < 10; i++) // hunger (should this be replaced/removed?)
			renderer->quads[num_quads++] = { vec2(.7 - (i * .06), -.63), scale / 12.f, vec3(.513, .309, .086) };

		// crosshair
		renderer->quads[num_quads++] = { {}, scale / 150.f, vec3(.5) };
	}

	if (selected_index >= 0 && screen)
	{
		renderer->icons[selected_index].position = vec2(mouse.norm_x, mouse.norm_y);
	}

	update(renderer->quad_mesh, MAX_GUI_QUADS * sizeof(Quad_Drawable), (byte*)renderer->quads);
	update(renderer->icon_mesh, MAX_GUI_QUADS * sizeof(Icon_Drawable), (byte*)renderer->icons);
}
void draw(GUI_Renderer* renderer)
{
	bind(renderer->icon_shader);
	bind_texture(renderer->texture, 0);
	draw(renderer->icon_mesh, MAX_GUI_QUADS);

	bind(renderer->quad_shader);
	draw(renderer->quad_mesh, MAX_GUI_QUADS);
}