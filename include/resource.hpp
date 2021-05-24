// resource.hpp

#ifndef _RESOURCE_HPP
#define _RESOURCE_HPP

#include "common.hpp"
#include "mesh.hpp"
#include "image.hpp"

enum Texture_id {
	TEXTURE_EARTH = 0,
	TEXTURE_SUN,
	TEXTURE_ALIEN,
	TEXTURE_MOON,
	TEXTURE_MONSTER,

	MAX_TEXTURE,
};

enum Mesh_id {
	MESH_SPHERE,
	MESH_CUBE,
	MESH_MONSTER,

	MAX_MESH,
};

typedef struct Resources {
	Image images[MAX_TEXTURE];
	u32 image_count;

	Mesh meshes[MAX_MESH];
	u32 mesh_count;
} Resources;

extern const char* texture_path[];

extern const char* mesh_path[];

void resources_initialize(Resources* resources);

void resources_load(Resources* resources);

void resources_unload(Resources* resources);

#endif
