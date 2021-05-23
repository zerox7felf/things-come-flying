// diffuse.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;

out vec4 out_color;

uniform sampler2D tex;

void draw_surface_normal(vec3 color) {
	float brightness = 2.0f;
	out_color = vec4(brightness * color * surface_normal, 1);
}

void draw_texture() {
	out_color = texture(tex, texture_coord);
}

void main() {
	// draw_surface_normal(vec3(0.9, 0.25, 0.25));
	draw_texture();
}
