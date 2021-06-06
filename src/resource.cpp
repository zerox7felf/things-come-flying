// resource.cpp
// manager for loading/unloading static resources

#include "resource.hpp"

// Ugh loading takes ages. We ideally want to have a threaded resource loader, but we ain't got time to implement that.
// TODO(lucas): Implement threaded resource loader if time is on our side.
const char* shader_path[MAX_SHADER] = {
	"resource/shader/textured_phong",
    "resource/shader/skybox",
	"resource/shader/texture",
	"resource/shader/combine",
	"resource/shader/blur",
	"resource/shader/flare",
	"resource/shader/brightness_extract",
};

const char* texture_path[MAX_TEXTURE] = {
	"resource/texture/missing.png",
	"resource/texture/house_diffuse.png",
	"resource/texture/house_specular.png",
	"resource/texture/house_normal.png",
    
	"resource/texture/lensflare_01.png",
	"resource/texture/lensflare_02.png",
	"resource/texture/lensflare_03.png",
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
    "resource/mesh/monke.obj",
    "resource/mesh/monke_flat.obj",
    "resource/mesh/plane.obj",
    "resource/mesh/bent_plane.obj",
    "resource/mesh/house.obj",
    "resource/mesh/quad.obj",
    "resource/mesh/destroyer.obj",
    "resource/mesh/saturn_rings.obj"
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
