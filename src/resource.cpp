// resource.cpp
// manager for loading/unloading static resources

#include "resource.hpp"

const char* texture_path[MAX_TEXTURE] = {
	"resource/texture/earth_daymap.png",
	"resource/texture/sun.png",
	"resource/texture/alien.png",
	"resource/texture/moon.png",
	"resource/texture/monster.png",
	"resource/texture/green.png",
};

const char* skybox_path[MAX_SKYBOX] = {
	"resource/texture/skybox/px.png",
	"resource/texture/skybox/nx.png",
	"resource/texture/skybox/py.png",
	"resource/texture/skybox/ny.png",
	"resource/texture/skybox/pz.png",
	"resource/texture/skybox/nz.png",
};

const char* mesh_path[MAX_MESH] = {
	"resource/mesh/sphere.obj",
	"resource/mesh/cube.obj",
	"resource/mesh/monster.obj",
    "resource/mesh/monke.obj"
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

	for (u32 i = 0; i < MAX_SKYBOX; i++) {
		Image* image = &resources->skybox_images[i];
		const char* path = skybox_path[i];
		load_image_from_file(path, image);
		resources->skybox_count++;
	}

	for (u32 i = 0; i < MAX_MESH; i++) {
		Mesh* mesh = &resources->meshes[i];
		const char* path = mesh_path[i];
		load_mesh(path, mesh, 1 /* sort mesh */);
		resources->mesh_count++;
	}
}

void resources_unload(Resources* resources) {
	for (u32 i = 0; i < resources->image_count; i++) {
		Image* image = &resources->images[i];
		unload_image(image);
	}
	resources->image_count = 0;

	for (u32 i = 0; i < resources->skybox_count; i++) {
		Image* image = &resources->skybox_images[i];
		unload_image(image);
	}
	resources->skybox_count = 0;

	for (u32 i = 0; i < resources->mesh_count; i++) {
		Mesh* mesh = &resources->meshes[i];
		unload_mesh(mesh);
	}
	resources->mesh_count = 0;
}
