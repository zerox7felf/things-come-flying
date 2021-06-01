#version 330 core

in vec4 vertex;
out vec2 texture_coord;

uniform mat4 perspective;
uniform mat4 orthogonal;
uniform mat4 view;
uniform mat4 model;

vec3 flare_source = vec3(0,0,0);

void main() {
	texture_coord = vec2(vertex.z, 1 - vertex.w);
    vec2 flare_source_screenspace = (perspective * view * vec4(flare_source, 1)).xy;
    vec2 flare_sprite_center = 0.5 * (vec2(0.5, 0.5) - flare_source_screenspace); // Halfway on the path between the source and the center of the screen
    vec4 vertex_pos = orthogonal * model * vec4(vertex.xy, 0, 1);
    gl_Position = vec4(
        //(vertex.xy) - 0.5 + flare_sprite_center, // center xy around zero, and then move it to the center position
        (vertex_pos.xy / 4) + flare_source_screenspace,
        0, 1
    );
}
