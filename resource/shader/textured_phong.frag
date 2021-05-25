// textured_phong.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;
in vec3 viewspace_position;

out vec4 out_color;

uniform sampler2D obj_texture;
uniform float emission;
uniform float shininess;
uniform float specular_amplitude;

uniform vec3 light_position; // light position in *viewspace* (ofc. since all our shading calcs are done there)
uniform vec3 light_color;

// Calculates ambient brightness
float ambient(float strength) {
    return strength;
}

// Calculates diffuse brightness
float diffuse(vec3 light_dir) {
    float light_distance = length(light_dir);
    return max(dot(surface_normal, light_dir), emission);
}

// Calculates specular brightness
float specular(vec3 light_dir) {
    vec3 view_dir = normalize(-viewspace_position);
    vec3 reflection = reflect(-light_dir, surface_normal);
    return pow(max(dot(view_dir, reflection), 0), shininess);
}

void main() {
    vec3 obj_color = texture(obj_texture, texture_coord).rgb;
    vec3 light_dir = normalize(light_position - viewspace_position);
    out_color = vec4(obj_color * light_color * (ambient(0.01) + diffuse(light_dir) + specular(light_dir)), 1);
}
