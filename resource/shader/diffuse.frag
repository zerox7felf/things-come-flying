// diffuse.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;
in vec3 frag_position;

out vec4 out_color;

uniform sampler2D tex;
uniform float emission;
uniform float shininess;
uniform vec3 camera_pos;

vec3 light_position = vec3(0, 0, 0);
vec3 light_color = vec3(1, 1, 1);

vec3 draw_surface_normal(vec3 color) {
	float brightness = 2.0f;
	return brightness * color * surface_normal;
}

vec3 draw_texture() {
	return texture(tex, texture_coord).rgb;
}

vec3 diffuse(float emit, vec3 light_delta, vec3 tex_color) {
	float light_distance = length(light_delta);
	float brightness = dot(surface_normal, normalize(light_delta));
	brightness = clamp(brightness, emit, 1);
	return brightness * light_color * tex_color;
}

vec3 gloss(float shine, vec3 light_delta, vec3 tex_color) {
    vec3 reflection = normalize(2 * dot(light_delta, surface_normal) * surface_normal - normalize(light_delta));
    float brightness = pow(dot(reflection, normalize(camera_pos)), shine);
    brightness = clamp(brightness, 0, 1);
    return brightness * light_color * tex_color;
}

vec3 ambient(vec3 tex_color) {
	float ambient_strength = 0.01f;
	return ambient_strength * tex_color;
}

void main() {
	vec3 light_delta = light_position - frag_position;
    vec3 tex_color = draw_texture();

	out_color = clamp(
        vec4(
            ambient(tex_color) +
            diffuse(emission, light_delta, tex_color) +
            gloss(shininess, light_delta, tex_color),
            1
        ), 0, 1
    );
}
