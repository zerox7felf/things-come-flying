// mesh.hpp

#ifndef _MESH_H
#define _MESH_H

#include "common.hpp"

typedef struct Mesh {
	v3* vertices;
	u32 vertex_count;

	u32* vertex_indices;
	u32 vertex_index_count;
} Mesh;

i32 load_mesh(const char* path, Mesh* mesh);

void unload_mesh(Mesh* mesh);

#endif
