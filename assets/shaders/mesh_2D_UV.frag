#version 420 core

struct VS_OUT
{
	vec2 tex_coords;
};

in VS_OUT vs_out;

layout (location = 0) out vec4 frag_color;

layout (binding = 0) uniform sampler2D texture_sampler;

void main()
{
	if(texture(texture_sampler, vs_out.tex_coords).rgb == vec3(0))
		discard;
	else
		frag_color = vec4(texture(texture_sampler, vs_out.tex_coords).rgb, 1);
}