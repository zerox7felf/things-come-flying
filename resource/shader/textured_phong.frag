// textured_phong.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;
in vec3 viewspace_position;

out vec4 out_color;

uniform sampler2D obj_texture0;
uniform sampler2D obj_texture1;

uniform float texture_mix;
uniform float ambient_amp;
uniform float diffuse_amp;
uniform float specular_amp;
uniform float shininess;

uniform vec3 light_position; // light position in *viewspace* (ofc. since all our shading calcs are done there)
uniform vec3 light_color;

void main() {
    vec3 interp_surface_normal = normalize(surface_normal);
    vec3 obj_color = texture(obj_texture0, texture_coord).rgb + (texture_mix * texture(obj_texture1, texture_coord).rgb);

    vec3 light_dir = normalize(light_position - viewspace_position);
    float diffuse = max(dot(interp_surface_normal, light_dir), 0) * diffuse_amp;

    vec3 view_dir = normalize(-viewspace_position);
    vec3 reflection = normalize(reflect(-light_dir, interp_surface_normal));
    float specular = pow(max(dot(view_dir, reflection), 0), shininess) * specular_amp;

    out_color = vec4(obj_color * light_color * (ambient_amp + diffuse + specular), 1);
}
