#version 330 core

struct VS_OUT
{
	vec3 normal;   // normal vector
	vec3 frag_pos; // position of this pixel in world space
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 proj_view;

out VS_OUT vs_out;

void main()
{
	vs_out.normal   = normal;
	vs_out.frag_pos = position;

	gl_Position = proj_view * vec4(vs_out.frag_pos, 1.0);
}