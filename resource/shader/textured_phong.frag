// textured_phong.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;
in vec3 viewspace_position;

out vec4 out_color;

// uniform sampler2D obj_texture0;
// uniform vec2 offset0;	// Texture uv offset uniforms are used to be able to animate the textures

uniform sampler2D color_map;
uniform vec2 color_map_offset;

uniform sampler2D ambient_map;
uniform vec2 ambient_map_offset;

// uniform sampler2D specular_map;
// uniform vec2 specular_map_offset;

uniform sampler2D obj_texture1;
uniform vec2 offset1;

uniform float texture_mix;
uniform float ambient_amp;
uniform float diffuse_amp;
uniform float specular_amp;
uniform float shininess;

uniform vec3 light_position; // light position in *viewspace* (ofc. since all our shading calcs are done there)
uniform vec3 light_color;

void main() {
    float frag_ambient_amp = ambient_amp;
    if (ambient_amp == -1) {
        frag_ambient_amp = texture(ambient_map, texture_coord + ambient_map_offset).r;
    }

    vec3 interp_surface_normal = normalize(surface_normal);
    vec3 obj_color = texture(color_map, texture_coord + color_map_offset).rgb + (texture_mix * texture(obj_texture1, texture_coord + offset1).rgb);

    vec3 light_dir = normalize(light_position - viewspace_position);
    float diffuse = max(dot(interp_surface_normal, light_dir), 0) * diffuse_amp;

    vec3 view_dir = normalize(-viewspace_position);
    vec3 reflection = normalize(reflect(-light_dir, interp_surface_normal));
    float specular = pow(max(dot(view_dir, reflection), 0), shininess) * specular_amp;

    out_color = vec4(obj_color * light_color * (frag_ambient_amp + diffuse + specular), 1);
}
