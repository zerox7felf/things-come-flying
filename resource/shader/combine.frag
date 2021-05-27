// combine.frag

#version 330 core

in vec2 texture_coord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture0;	// Processed image
uniform sampler2D texture1;	// Original image
uniform float mix;

vec4 color = vec4(0);

void main() {
	color = texture(texture1, texture_coord) + (mix * texture(texture0, texture_coord));
	// color = texture(texture0, texture_coord);

	out_color = color;
}
