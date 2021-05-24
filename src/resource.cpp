// resource.cpp
// manager for loading/unloading static resources

#include "resource.hpp"

const char* texture_path[MAX_TEXTURE] = {
	"resource/texture/2k_earth_daymap.png",
	"resource/texture/8k_sun.png",
	"resource/texture/4k_alien.png",
	"resource/texture/4k_moon.png",
	"resource/texture/monster.png",
};

const char* mesh_path[MAX_MESH] = {
	"resource/mesh/sphere.obj",
	"resource/mesh/cube.obj",
	"resource/mesh/monster.obj",
};

void resources_initialize(Resources* resources) {
	resources->image_count = 0;
	resources->mesh_count = 0;
}

void resources_load(Resources* resources) {
	for (u32 i = 0; i < MAX_TEXTURE; i++) {
		Image* image = &resources->images[i];
		const char* path = texture_path[i];
		load_image_from_file(path, image);
		resources->image_count++;
	}

	for (u32 i = 0; i < MAX_MESH; i++) {
		Mesh* mesh = &resources->meshes[i];
		const char* path = mesh_path[i];
		load_mesh(path, mesh, 1 /* sort mesh */);
		resources->mesh_count++;
	}
}

void resources_unload(Resources* resources) {
	for (u32 i = 0; i < MAX_TEXTURE; i++) {
		Image* image = &resources->images[i];
		unload_image(image);
	}

	for (u32 i = 0; i < MAX_MESH; i++) {
		Mesh* mesh = &resources->meshes[i];
		unload_mesh(mesh);
	}
}
