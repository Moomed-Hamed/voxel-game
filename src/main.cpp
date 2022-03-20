#include "player.h"

int main()
{
	Window   window = {};
	Mouse    mouse  = {};
	Keyboard keys   = {};

	init(&window, 1920, 1080, "voxel game");
	init(&keys);

	Particle_Emitter* emitter = Alloc(Particle_Emitter, 1);
	Particle_Renderer* particle_renderer = Alloc(Particle_Renderer, 1);
	init(particle_renderer);

	Player* player = Alloc(Player, 1);
	init(player);

	World* world = Alloc(World, 1);
	init(world, player->eyes.position);

	World_Renderer* world_renderer = Alloc(World_Renderer, 1);
	init(world_renderer);

	GUI_Renderer* gui = Alloc(GUI_Renderer, 1);
	init(gui);

	Audio pops[4] = {
		load_audio("assets/audio/pop_0.audio"),
		load_audio("assets/audio/pop_1.audio"),
		load_audio("assets/audio/pop_2.audio"),
		load_audio("assets/audio/block.audio")
	};

	G_Buffer g_buffer = make_g_buffer(window);
	Shader lighting_shader = make_lighting_shader();
	mat4 proj = perspective(FOV, (float)window.screen_width / window.screen_height, 0.1f, DRAW_DISTANCE);

	// frame timer
	float frame_time = 1.f / 60;
	int64 target_frame_milliseconds = frame_time * 1000.f; // seconds * 1000 = milliseconds
	Timestamp frame_start = get_timestamp(), frame_end;

	while (1)
	{
		update(window);
		update(&mouse, window);
		update(&keys, window);

		// game updates
		update(player, world, keys, mouse, frame_time, emitter, pops, gui->icons);
		update(emitter, frame_time, vec3(0));
		update(world, player->eyes, mouse, frame_time, player->items, pops);

		// renderer updates
		update(particle_renderer , emitter);
		update(world_renderer, world, frame_time);
		update(gui, mouse, player->items, player->action, player->selected_item, player->opened_items);

		if (player->status != STATUS_IN_MENU)
			disable_cursor(window);
		else
			enable_cursor(window);

		// geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer.FBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mat4 proj_view = proj * lookAt(player->eyes.position, player->eyes.position + player->eyes.front, player->eyes.up);
		
		draw(particle_renderer, proj_view);
		draw(world_renderer   , proj_view, frame_time);

		// lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bind(lighting_shader);
		set_vec3(lighting_shader, "view_pos", player->eyes.position);
		draw(g_buffer);
		draw(gui);

		// frame time
		frame_end = get_timestamp();
		int64 milliseconds_elapsed = calculate_milliseconds_elapsed(frame_start, frame_end);

		//print("frame time: %02d ms | fps: %06f\n", milliseconds_elapsed, 1000.f / milliseconds_elapsed);
		if (target_frame_milliseconds > milliseconds_elapsed) // frame finished early
			os_sleep(target_frame_milliseconds - milliseconds_elapsed);
		
		frame_start = frame_end;
	}

	shutdown_window();
	return 0;
}