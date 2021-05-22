// diffuse.vert

#version 330 core

in vec3 position;
in vec2 uv;
in vec3 normal;

out vec3 surface_normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	surface_normal = mat3(transpose(inverse(model))) * normal;	// I have no idea how this works, but here be surface normal
	gl_Position = projection * view * model * vec4(position, 1);
}
