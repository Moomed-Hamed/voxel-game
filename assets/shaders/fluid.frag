#version 330 core

struct VS_OUT
{
	vec3 normal;   // normal vector
	vec3 frag_pos; // position of this pixel in world space
	float height;
};

in VS_OUT vs_out;

layout (location = 0) out vec4 frag_position;
layout (location = 1) out vec4 frag_normal;
layout (location = 2) out vec4 frag_albedo;

void main()
{
	vec3 white = vec3(1,1,1) * (vs_out.height * .05);
	white += vec3(0,1,1);

	frag_position = vec4(vs_out.frag_pos, 0);
	frag_normal   = vec4(vs_out.normal, 0);
	frag_albedo   = vec4(white,0);
}