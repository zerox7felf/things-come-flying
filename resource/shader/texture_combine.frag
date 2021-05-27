// texture_combine.frag

#version 330 core

in vec2 texture_coord;

layout (location = 0) out vec4 out_color;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
	vec4 color = texture(texture0, texture_coord) + texture(texture1, texture_coord);
	// vec4 color_modified = vec4(color.b, color.r, color.g, 1);
	// out_color = color_modified;
    out_color = color;
}
