// diffuse.vert

#version 330 core
in vec3 position;
in vec2 uv;
in vec3 normal;

out vec3 surface_normal;
out vec2 texture_coord;
out vec3 viewspace_position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	surface_normal = normalize(mat3(transpose(inverse(view * model))) * normal);	//TODO: move normal transform to uniform
	texture_coord = vec2(uv.x, 1 - uv.y);                                           // Flip Y so blender uv-s work
	viewspace_position = vec3(view * model * vec4(position, 1));                    //TODO: VM to uniform
	gl_Position = projection * view * model * vec4(position, 1);                    //TODO: PVM to uniform
}
