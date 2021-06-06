// textured_phong.frag

#version 330 core

in vec3 surface_normal;
in vec2 texture_coord;
in vec3 viewspace_position;
in mat3 TBN;

out vec4 out_color;

// uniform sampler2D obj_texture0;
// uniform vec2 offset0;	// Texture uv offset uniforms are used to be able to animate the textures

uniform sampler2D color_map;
uniform vec2 color_map_offset;

uniform sampler2D ambient_map;
uniform vec2 ambient_map_offset;

uniform sampler2D specular_map;
uniform vec2 specular_map_offset;

uniform sampler2D diffuse_map;
uniform vec2 diffuse_map_offset;

uniform sampler2D normal_map;
uniform vec2 normal_map_offset;

uniform sampler2D obj_texture1;
uniform vec2 offset1;

uniform float texture_mix;
uniform float ambient_amp;
uniform float diffuse_amp;
uniform float specular_amp;
uniform float normal_amp; // Just a flag for if we have a normal map or not.
uniform float shininess;

uniform mat4 V;

#define MAX_LIGHTS 64
struct Point_light {
    vec3 position; // position in view space
    vec3 color;
    float ambient;
    float falloff_linear;
    float falloff_quadratic;
};
uniform Point_light point_lights[MAX_LIGHTS];
uniform int num_point_lights;

struct Sun_light {
    vec3 angle; // UN-normalized direction vector, world space
    vec3 color;
    float ambient;
    float falloff_linear;
    float falloff_quadratic;
};
uniform Sun_light sun_lights[MAX_LIGHTS];
uniform int num_sun_lights;

void main() {
    vec3 obj_color = texture(color_map, texture_coord + color_map_offset).rgb + (texture_mix * texture(obj_texture1, texture_coord + offset1).rgb);

    vec3 frag_ambient_amp = obj_color * ambient_amp;
    if (ambient_amp == -1) {
        frag_ambient_amp = texture(ambient_map, texture_coord + ambient_map_offset).rgb;
    }

    vec3 frag_diffuse_amp = obj_color * diffuse_amp;
    if (diffuse_amp == -1) {
        frag_diffuse_amp = texture(diffuse_map, texture_coord + diffuse_map_offset).rgb;
    }

    vec3 frag_specular_amp = obj_color * specular_amp;
    if (specular_amp == -1) {
        frag_specular_amp = texture(specular_map, texture_coord + specular_map_offset).rgb;
    }

    vec3 interp_surface_normal = normalize(surface_normal);
    if (normal_amp == -1) {
        interp_surface_normal = texture(normal_map, texture_coord + normal_map_offset).rgb * 2 - 1;
        interp_surface_normal = normalize(TBN * interp_surface_normal);
    }

    vec3 view_dir = normalize(-viewspace_position);
    vec3 out_rgb = vec3(0,0,0);
    for (int i = 0; i < num_point_lights; i++) {
        Point_light light = point_lights[i];
        vec3 light_pos = (V * vec4(light.position, 1.0f)).xyz;

        float light_distance = length(light_pos - viewspace_position);
        float falloff = 1.0f / (1 + light.falloff_linear * light_distance + light.falloff_quadratic * (light_distance * light_distance));

        vec3 light_dir = normalize(light_pos - viewspace_position);
        vec3 reflection = normalize(reflect(-light_dir, interp_surface_normal));

        vec3 diffuse = max(dot(interp_surface_normal, light_dir), 0) * frag_diffuse_amp;
        vec3 specular = pow(max(dot(view_dir, reflection), 0), shininess) * frag_specular_amp;
        out_rgb += vec3(light.color * falloff * (frag_ambient_amp * light.ambient + diffuse + specular));
    }
    for (int i = 0; i < num_sun_lights; i++) {
        Sun_light light = sun_lights[i];
        vec3 light_dir = normalize(V * vec4(-light.angle, 0.0f)).xyz;
        vec3 reflection = normalize(reflect(-light_dir, interp_surface_normal));

        vec3 diffuse = max(dot(interp_surface_normal, light_dir), 0) * frag_diffuse_amp;
        vec3 specular = pow(max(dot(view_dir, reflection), 0), shininess) * frag_specular_amp;
        out_rgb += vec3(light.color * (frag_ambient_amp * light.ambient + diffuse + specular));
    }

    out_color = vec4(out_rgb, 1);
    //out_color = vec4(light_color * (frag_ambient_amp + diffuse + specular), 1);
}
