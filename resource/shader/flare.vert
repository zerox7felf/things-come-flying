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
    vec4 vertex_pos = orthogonal * model * vec4(vertex.xy, 0, 1);
    vec4 flare_source_screenspace = perspective * view * vec4(flare_source, 1);
    gl_Position = vec4(
        vertex_pos.x / 8 - 1,
        vertex_pos.y / 8 - 1,
        //(mod(flare_source_screenspace.y + 1, 2) - 1) / 8,
        0, 1
    );
}
