// ground.vert

#version 330 core
in vec3 position;
in vec2 uv;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;

out vec3 surface_normal;
out vec2 texture_coord;
out vec3 viewspace_position;
out mat3 TBN;

uniform mat4 P;
uniform mat4 VM;
uniform mat4 PVM;
uniform mat4 VM_normal;

void main() {
	texture_coord = vec2(uv.x, 1 - uv.y); // Flip Y so blender's UVs work

	viewspace_position = (VM * vec4(position, 1)).xyz;
    float dist_to_cam = length(viewspace_position.xz); // only count distance in x and z
    vec3 new_world_pos = position;
    new_world_pos.y -= (dist_to_cam * dist_to_cam) / 800;
	viewspace_position = (VM * vec4(new_world_pos, 1)).xyz;

	surface_normal = normalize(mat3(VM_normal) * normal);
    vec3 viewspace_tangent = normalize(mat3(VM_normal) * tangent);
    vec3 viewspace_bitangent = normalize(mat3(VM_normal) * bitangent);
    TBN = mat3(viewspace_tangent, viewspace_bitangent, surface_normal);

	gl_Position = PVM * vec4(new_world_pos, 1);
}
