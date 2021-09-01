#include "world.h"

#define TARGET_FRAMES_PER_SECOND ((float)120)
#define DRAW_DISTANCE 512.0f
#define FOV ToRadians(55.0f)

int main()
{
	Window   window = {};
	Mouse    mouse  = {};
	Keyboard keys   = {};

	init_window(&window, 1920, 1080, "voxel game");
	init_keyboard(&keys);

	Camera camera = {};
	camera.position = vec3(36);

	Item* items = Alloc(Item, MAX_ITEMS);
	Item_Renderer* item_renderer = Alloc(Item_Renderer, 1);
	init(item_renderer);

	Enemy* enemies = Alloc(Enemy, MAX_ENEMIES);
	Enemy_Renderer* enemy_renderer = Alloc(Enemy_Renderer, 1);
	init(enemy_renderer);

	World* world = Alloc(World, 1);
	init(world, camera.position);

	World_Renderer* world_renderer = Alloc(World_Renderer, 1);
	init(world_renderer);

	Crosshair_Renderer* gui = Alloc(Crosshair_Renderer, 1);
	init(gui);

	Tool_Renderer* axe_renderer = Alloc(Tool_Renderer, 1);
	init(axe_renderer);

	Sky_Renderer* sky_renderer = Alloc(Sky_Renderer, 1);
	init(sky_renderer);

	Particle_Emitter* emitter = Alloc(Particle_Emitter, 1);
	Particle_Renderer* particle_renderer = Alloc(Particle_Renderer, 1);
	init(particle_renderer);

	// audio
	Audio pops[3] = {};
	pops[0] = load_audio("assets/audio/pop_0.audio");
	pops[1] = load_audio("assets/audio/pop_1.audio");
	pops[2] = load_audio("assets/audio/pop_2.audio");

	G_Buffer g_buffer = {};
	init_g_buffer(&g_buffer, window);
	Shader lighting_shader = make_lighting_shader();
	mat4 proj = glm::perspective(FOV, (float)window.screen_width / window.screen_height, 0.1f, DRAW_DISTANCE);

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

		if (keys.W.is_pressed) camera_update_pos(&camera, DIR_FORWARD , 15 * frame_time);
		if (keys.S.is_pressed) camera_update_pos(&camera, DIR_BACKWARD, 15 * frame_time);
		if (keys.A.is_pressed) camera_update_pos(&camera, DIR_LEFT    , 15 * frame_time);
		if (keys.D.is_pressed) camera_update_pos(&camera, DIR_RIGHT   , 15 * frame_time);
		if (camera.position.x < 32) camera.position.x = 32;
		if (camera.position.z < 32) camera.position.z = 32;

		if (keys.U.is_pressed) set_vec3(lighting_shader, "light_positions[0]", camera.position);
		if (keys.I.is_pressed) set_vec3(lighting_shader, "light_positions[1]", camera.position);
		if (keys.O.is_pressed) set_vec3(lighting_shader, "light_positions[2]", camera.position);
		if (keys.P.is_pressed) set_vec3(lighting_shader, "light_positions[3]", camera.position);

		camera_update_dir(&camera, mouse.dx, mouse.dy, frame_time);

		// game updates
		update(emitter, frame_time, vec3(0));
		update(items, world->active_chunks, camera, frame_time, pops);
		update(world, keys, mouse, camera, emitter, items);

		enemies[0].position = vec3(40.5, 27, 48.5);

		// renderer updates
		update(gui);
		update_renderer(world_renderer, world, frame_time);
		update_renderer(axe_renderer, camera, mouse.norm_dx, frame_time);
		update_renderer(sky_renderer, camera.position);
		update_renderer(particle_renderer, emitter);
		update_renderer(item_renderer, items, frame_time);
		update_renderer(enemy_renderer, enemies, frame_time);

		mat4 proj_view = proj * lookAt(camera.position, camera.position + camera.front, camera.up);

		// Geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer.FBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw(gui);
		draw(axe_renderer     , proj_view);
		draw(sky_renderer     , proj_view);
		draw(particle_renderer, proj_view);
		draw(item_renderer    , proj_view);
		draw(enemy_renderer   , proj_view);
		draw(world_renderer, frame_time, proj_view);

		// Lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bind(lighting_shader);
		set_vec3(lighting_shader, "view_pos", camera.position);
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