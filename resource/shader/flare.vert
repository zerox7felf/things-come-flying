#version 330 core

in vec4 vertex;
out vec2 texture_coord;
out float flare_opacity;

uniform mat4 projection;
uniform mat4 orthogonal;
uniform mat4 view;
uniform mat4 model;
uniform mat4 reverse_model;

uniform float flare_position; // How far along the flare vector should we be positioned?
uniform float flare_size;
uniform vec3 flare_source;

void main() {
    //vec3 flare_source = vec3(0,0,0);

	texture_coord = vec2(vertex.z, 1 - vertex.w);
    vec4 vertex_pos = orthogonal * model * vec4(vertex.xy, 0, 1);
    vec4 flare_source_screenspace = projection * view * vec4(flare_source, 1);

    if (flare_source_screenspace.w > 0) {
        // To account for perspective:
        float flare_source_x = flare_source_screenspace.x / flare_source_screenspace.w;
        float flare_source_y = flare_source_screenspace.y / flare_source_screenspace.w;
        vec2 flare_vector = vec2(flare_source_x, flare_source_y);

        flare_opacity = 1 - ((length(flare_vector) * length(flare_vector)) / 0.9f);

        gl_Position = vec4(
            vertex_pos.xy * flare_size + flare_vector * flare_position,
            0, 1
        );
    } else {
        // Outside of screen. w <= 0 which means that it is not visible.
        gl_Position = vec4(2, 2, 0, 1);
        flare_opacity = 0;
    }
}
