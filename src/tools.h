#include "items.h"

// rendering

struct Tool_Drawable
{
	vec3 position;
	mat3 rotation;
};

struct Tool_Renderer
{
	Drawable_Mesh_UV mesh;
	Shader shader;
};

void init(Tool_Renderer* renderer)
{
	load(&renderer->mesh, "assets/meshes/sword.mesh_uv", sizeof(Tool_Drawable));
	mesh_add_attrib_vec3(3, sizeof(Tool_Drawable), 0 * sizeof(vec3)); // world pos
	mesh_add_attrib_mat3(4, sizeof(Tool_Drawable), 1 * sizeof(vec3)); // rotation

	renderer->mesh.texture_id  = load_texture("assets/textures/palette.bmp");
	renderer->mesh.material_id = load_texture("assets/textures/materials.bmp");

	load(&(renderer->shader), "assets/shaders/transform/mesh_uv.vert", "assets/shaders/mesh_uv.frag");
}
void update_renderer(Tool_Renderer* renderer, Camera cam, float turn, float dtime)
{
	vec3 pos   = cam.position;
	vec3 front = cam.front;
	vec3 right = cam.right;
	vec3 up    = cam.up;
	vec3 offset = (front * 1.5f) + (.8f * cross(up, -1.f * front)) - (up * .3f);

	static float turn_amount = 0; turn_amount += turn;
	if (turn_amount >  .1) turn_amount = .1;
	if (turn_amount < -.1) turn_amount = -.1;

	vec3 look = -1.f * lerp(front, right, -.12 + turn_amount);
	turn_amount *= dtime;

	static float action_timer = 0; action_timer += dtime;
	if (action_timer > 1) action_timer = 0;
	mat3 rot = rotate(PI * bounce(action_timer, -.7, .5, -.4) / 6, right * -1.f);

	Tool_Drawable tool = {};
	tool.position = pos + offset;
	tool.rotation = mat3(.85) * rot * point_at(look, up);

	static float time = 0; time += dtime;
	if (1)
	{
		tool.position += up    * (.01f  * sin(time * 2.6f));
		tool.position += right * (.005f * sin(time * 1.6f));
	}

	update(renderer->mesh, sizeof(Tool_Drawable), (byte*)&tool);
}
void draw(Tool_Renderer* renderer, mat4 proj_view)
{
	bind(renderer->shader);
	set_mat4(renderer->shader, "proj_view", proj_view);
	bind_texture(renderer->mesh);
	draw(renderer->mesh);
}