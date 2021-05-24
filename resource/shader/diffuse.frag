// diffuse.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;
in vec3 frag_position;

out vec4 out_color;

uniform sampler2D tex;
uniform float emission;

vec3 light_position = vec3(0, 0, 0);
vec3 light_color = vec3(1, 1, 1);

vec3 draw_surface_normal(vec3 color) {
	float brightness = 2.0f;
	return brightness * color * surface_normal;
}

vec3 draw_texture() {
	return texture(tex, texture_coord).rgb;
}

vec3 diffuse(float emit) {
	vec3 normal = normalize(surface_normal);
	vec3 light_delta = light_position - frag_position;
	float light_distance = length(light_delta);
	float brightness = dot(normal, light_delta) / (light_distance * length(normal));
	brightness = clamp(brightness, emit, 1);
	return brightness * light_color;
}

vec3 ambient() {
	float ambient_strength = 0.01f;
	return ambient_strength * light_color;
}

void main() {
	out_color = vec4(ambient() + diffuse(emission) * draw_texture(), 1);
}
