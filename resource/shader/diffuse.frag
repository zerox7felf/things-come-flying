// diffuse.frag

#version 330 core

in vec3 surface_normal;

out vec4 out_color;

void main() {
	vec3 color = vec3(0.9, 0.25, 0.25);
	float brightness = 2.0f;
	out_color = vec4(brightness * color * surface_normal, 1);
}
