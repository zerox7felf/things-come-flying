// texture.frag

#version 330 core

in vec2 texture_coord;

layout (location = 0) out vec4 out_color;

uniform sampler2D tex;

void main() {
	vec4 color = texture(tex, texture_coord);
	out_color = color;
}
