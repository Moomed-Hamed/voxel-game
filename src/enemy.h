#include "chunk.h"

#define MAX_ENEMIES 4

struct Enemy
{
	vec3 position;
	vec3 direction;
	float health;
};

void spawn(Enemy* enemies, vec3 pos)
{

}

// rendering

struct Enemy_Drawable
{
	vec3 position;
	mat3 rotation;
};

struct Enemy_Renderer
{
	Enemy_Drawable enemies[MAX_ENEMIES];
	Drawable_Mesh_Anim_UV mesh;
	Shader shader;
	Animation animation;
	mat4 current_pose[MAX_ANIMATED_BONES];
};

void init(Enemy_Renderer* renderer)
{
	load(&renderer->mesh, "assets/meshes/enemy.mesh_anim", sizeof(renderer->enemies));
	mesh_add_attrib_vec3(5, sizeof(Enemy_Drawable), 0); // world pos
	mesh_add_attrib_mat3(6, sizeof(Enemy_Drawable), sizeof(vec3)); // rotation

	renderer->mesh.texture_id  = load_texture("assets/textures/palette.bmp");
	renderer->mesh.material_id = load_texture("assets/textures/materials.bmp");

	load(&(renderer->shader), "assets/shaders/transform/mesh_anim_uv.vert", "assets/shaders/mesh_uv.frag");

	load(&renderer->animation, "assets/animations/enemy.anim"); // animaiton keyframes
	GLint skeleton_id = glGetUniformBlockIndex(renderer->shader.id, "skeleton");
	glUniformBlockBinding(renderer->shader.id, skeleton_id, 1); // this is a hack & needs more attention
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, renderer->mesh.UBO);
}
void update_renderer(Enemy_Renderer* renderer, Enemy* enemies, float dtime)
{
	renderer->enemies[0].position = enemies[0].position;
	renderer->enemies[0].rotation = mat3(1);

	update_pose(&renderer->animation, renderer->current_pose, 2, 3, 0);
	update(renderer->mesh, renderer->animation.num_bones, renderer->current_pose, sizeof(renderer->enemies), (byte*)renderer->enemies);
}
void draw(Enemy_Renderer* renderer, mat4 proj_view)
{
	bind(renderer->shader);
	set_mat4(renderer->shader, "proj_view", proj_view);
	bind_texture(renderer->mesh);
	glBindBuffer(GL_UNIFORM_BUFFER, 1);
	draw(renderer->mesh);
}