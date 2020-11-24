#include "player.h"

#define TARGET_FRAMES_PER_SECOND ((float)120)
#define DRAW_DISTANCE 512.0f
#define FOV ToRadians(45.0f)

int main()
{
	// byte* game_memory = Virtual Alloc

	Window   window = {};
	Mouse    mouse  = {};
	Keyboard keys   = {};

	init_window(&window, 1920, 1080, "voxel game");
	init_keyboard(&keys);

	Player_One* player = Alloc(Player_One, 1); //Camera camera = { vec3(63,32,63) };
	ready_player_one(player);

	G_Buffer g_buffer = {};
	init_g_buffer(&g_buffer, window);
	Shader lighting_shader = make_lighting_shader();
	mat4 proj = glm::perspective(FOV, (float)window.screen_width / window.screen_height, 0.1f, DRAW_DISTANCE);

	World* world = Alloc(World, 1);
	init(world, player->position);

	Chunk_Renderer* chunk_renderer = Alloc(Chunk_Renderer, 1);
	init(chunk_renderer);

	Loose_Item* items = Alloc(Loose_Item, MAX_LOOSE_ITEMS);
	Loose_Item_Renderer* item_renderer = Alloc(Loose_Item_Renderer, 1);
	init(item_renderer);

	Crop* crops = Alloc(Crop, MAX_CROPS);
	Crop_Renderer* crop_renderer = Alloc(Crop_Renderer, 1);
	init(crop_renderer);

	// automata
	Cell_Arena* cell_arena = Alloc(Cell_Arena, 1);
	init(cell_arena, uvec3(20, 16, 20));
	Cell_Renderer* cell_renderer = Alloc(Cell_Renderer, 1);
	init(cell_renderer);

	// frame timer
	float frame_time = 1.f / 60;
	int64 target_frame_milliseconds = frame_time * 1000.f;
	Timestamp frame_start = get_timestamp(), frame_end;

	while (1)
	{
		update_window(window);
		update_mouse(&mouse, window);
		update_keyboard(&keys, window);

		if (keys.ESC.is_pressed) break;

		player_update_dir(player, mouse.dx, mouse.dy);
		if (keys.W.is_pressed) player_move(player, DIR_FORWARD , 10 * frame_time);
		if (keys.A.is_pressed) player_move(player, DIR_BACKWARD, 10 * frame_time);
		if (keys.S.is_pressed) player_move(player, DIR_LEFT    , 8  * frame_time);
		if (keys.D.is_pressed) player_move(player, DIR_RIGHT   , 8  * frame_time);
		if (keys.SPACE.is_pressed && !keys.SPACE.was_pressed) player->velocity.y = 15;
		player_update_pos(player, world->chunks, frame_time);

		if (mouse.left_button.is_pressed && !mouse.left_button.was_pressed)
		{
			vec3 break_pos;
			uint16 broken_block = world_break_block_raycast(world->chunks, player->position, player->front, &break_pos);
			//if (broken_block) add_loose_item(items, Item{ BLOCK_ITEM , broken_block, 1 }, break_pos, vec3{ 0, 2, 0 });
		}

		if (mouse.right_button.is_pressed && !mouse.right_button.was_pressed)
		{
			world_place_block_raycast(world->chunks, player->position, player->front, BLOCK::DIRT);
		}

		if (keys.F.is_pressed) // flashlight
		{
			bind(lighting_shader);
			set_vec3(lighting_shader, "spt_light.position", player->position);
			set_vec3(lighting_shader, "spt_light.direction", player->front);
		}
		else if (keys.X.is_pressed)
		{
			bind(lighting_shader);
			set_vec3(lighting_shader, "spt_light.position", vec3(0, 0, 0));
			set_vec3(lighting_shader, "spt_light.direction", vec3(0, -1, 0));
		}

		if (keys.E.is_pressed && !keys.E.was_pressed)
		{
			uvec3 crop_coords = {};
			BlockID b = world_place_block_raycast(world->chunks, player->position, player->front, BLOCK::AIR, &crop_coords);
			if (b == BLOCK::DIRT)
			{
				world_set_block(world->chunks, crop_coords, BLOCK::CROP);
				add_crop(crops, crop_coords, CROP_WHEAT);
			}
		}

		//update(world, camera.position);
		update(items, world->chunks, player->position, frame_time);
		update(crops, world->chunks, frame_time);
		update(cell_arena, frame_time);

		// rendering updates
		update_renderer(chunk_renderer, world->chunks);
		update_renderer(item_renderer , items, frame_time);
		update_renderer(crop_renderer , crops);
		update_renderer(cell_renderer , cell_arena);

		mat4 proj_view = proj * glm::lookAt(player->position, player->position + player->front, player->up);

		// Geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer.FBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bind(chunk_renderer->solid_shader);
		set_mat4(chunk_renderer->solid_shader, "proj_view", proj_view);
		bind_texture(chunk_renderer->solid_mesh, 3);

		draw(chunk_renderer->solid_mesh, chunk_renderer->num_solids);

		static float timer = 0; timer += frame_time;
		bind(chunk_renderer->fluid_shader);
		set_mat4(chunk_renderer->fluid_shader, "proj_view", proj_view);
		set_float(chunk_renderer->fluid_shader, "timer", timer);

		draw(chunk_renderer->fluid_mesh, chunk_renderer->num_fluids);

		bind(item_renderer->block_shader);
		set_mat4(item_renderer->block_shader, "proj_view", proj_view);
		bind_texture(item_renderer->block_mesh, 3);

		draw(item_renderer->block_mesh, item_renderer->num_blocks);

		bind(crop_renderer->crop_shader);
		set_mat4(crop_renderer->crop_shader, "proj_view", proj_view);

		draw(crop_renderer->crop_mesh, crop_renderer->num_crops);

		bind(cell_renderer->cell_shader);
		set_mat4(cell_renderer->cell_shader, "proj_view", proj_view);

		draw(cell_renderer->cell_mesh, cell_renderer->num_cells);

		// Lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bind(lighting_shader);
		set_vec3(lighting_shader, "view_pos", player->position);
		draw_g_buffer(g_buffer);

		//Frame Time
		frame_end = get_timestamp();
		int64 milliseconds_elapsed = calculate_milliseconds_elapsed(frame_start, frame_end);

		//print("frame time: %02d ms | fps: %06f\n", milliseconds_elapsed, 1000.f / milliseconds_elapsed);
		if (target_frame_milliseconds > milliseconds_elapsed) // frame finished early
		{
			os_sleep(target_frame_milliseconds - milliseconds_elapsed);
		}
		
		frame_start = frame_end;
	}

	shutdown_window();
	return 0;
}