// mesh.hpp

#ifndef _MESH_H
#define _MESH_H

#include "common.hpp"

typedef struct Mesh {
	v3* vertices;
	u32 vertex_count;

	u32* vertex_indices;
	u32 vertex_index_count;

	v2* uv;
	u32 uv_count;

	u32* uv_indices;
	u32 uv_index_count;

	v3* normals;
	u32 normal_count;

	u32* normal_indices;
	u32 normal_index_count;
} Mesh;

i32 mesh_sort_indices(Mesh* mesh);

i32 load_mesh(const char* path, Mesh* mesh, u8 sort_mesh);

void unload_mesh(Mesh* mesh);

#endif
