// skybox.vert

#version 330 core

in vec3 position;

out vec3 texture_coord;

uniform mat4 projection;
uniform mat4 view;

void main() {
	texture_coord = position;
	gl_Position = (projection * view * vec4(position, 1)).xyww;
}
