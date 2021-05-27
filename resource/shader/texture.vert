// texture.vert

#version 330 core

in vec4 vertex;

out vec2 texture_coord;

uniform mat4 projection;
uniform mat4 model;

void main() {
	texture_coord = vec2(vertex.z, 1 - vertex.w);
	gl_Position = projection * model * vec4(vertex.xy, 0, 1);
}
