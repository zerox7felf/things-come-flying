// diffuse.vert

#version 330 core

in vec3 position;
in vec2 uv;
in vec3 normal;

out vec3 surface_normal;
out vec2 texture_coord;
out vec3 frag_position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	surface_normal = mat3(transpose(inverse(model))) * normal;	// I have no idea how this works, but here be surface normal
	texture_coord = vec2(uv.x, 1 - uv.y);
	frag_position = vec3(model * vec4(position, 1));
	gl_Position = projection * view * model * vec4(position, 1);
}
