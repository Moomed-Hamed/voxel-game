#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 world_position;
layout (location = 4) in float tex_offset;

struct VS_OUT
{
	vec3 normal;   // normal vector
	vec3 frag_pos; // position of this pixel in world space
	vec2 tex_coord;
};

uniform mat4 proj_view;

out VS_OUT vs_out;

void main()
{
	vs_out.normal = normal;
	vs_out.frag_pos = position + world_position;
	vs_out.tex_coord = vec2(tex_coord.x + tex_offset, tex_coord.y);
	gl_Position = proj_view * vec4(vs_out.frag_pos, 1.0);
}
