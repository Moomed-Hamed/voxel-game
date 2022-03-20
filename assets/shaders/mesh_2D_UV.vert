#version 420 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 tex_coords;
layout (location = 2) in vec2 position;
layout (location = 3) in vec2 scale;
layout (location = 4) in vec2 tex_offset;

struct VS_OUT
{
	vec2 tex_coords;
};

out VS_OUT vs_out;

void main()
{
	vs_out.tex_coords = tex_coords + tex_offset;
	gl_Position = vec4((vertex * scale) + position, 0, 1.0);
}