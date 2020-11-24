#version 330 core

struct VS_OUT
{
	vec3 normal;   // normal vector
	vec3 frag_pos; // position of this pixel in world space
	float height;
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 world_pos;

uniform mat4 proj_view;
uniform float timer;

out VS_OUT vs_out;

void main()
{
	vs_out.normal   = normal;
	vs_out.frag_pos = position + world_pos;

	float x1 = (position.x + 1) + world_pos.x;
	float x2 = (position.z + 1) + world_pos.z;
	float factor  = sqrt( (x1 * x1) + (x2 * x2) );

	vs_out.frag_pos.y += (.05 * sin( (-timer *  3.14159) + (3.1415 * factor) )) - .1;
	vs_out.height = sin( (-timer * 3.14159) + (3.1415 * factor) );

	gl_Position = proj_view * vec4(vs_out.frag_pos, 1);
}