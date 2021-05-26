// resource.hpp

#ifndef _RESOURCE_HPP
#define _RESOURCE_HPP

#include "common.hpp"
#include "mesh.hpp"
#include "image.hpp"

enum Texture_id {
	TEXTURE_EARTH = 0,
	TEXTURE_EARTH_CLOUDS,
	TEXTURE_SUN,
	TEXTURE_ALIEN,
	TEXTURE_MOON,
	TEXTURE_GUY,
    TEXTURE_GREEN,

	MAX_TEXTURE,
};

enum Skybox_texture_id {
	SKYBOX_SPACE_PX = 0,
	SKYBOX_SPACE_NX,
	SKYBOX_SPACE_PY,
	SKYBOX_SPACE_NY,
	SKYBOX_SPACE_PZ,
	SKYBOX_SPACE_NZ,

	MAX_SKYBOX,
};

enum Cube_map_id {
	CUBE_MAP_SPACE,

	MAX_CUBE_MAP,
};

enum Mesh_id {
	MESH_SPHERE,
	MESH_CUBE,
	MESH_GUY,
    MESH_MONKE,

	MAX_MESH,
};

typedef struct Resources {
	Image images[MAX_TEXTURE];
	u32 image_count;

	Image skybox_images[MAX_SKYBOX];
	u32 skybox_count;

	Mesh meshes[MAX_MESH];
	u32 mesh_count;
} Resources;

extern const char* texture_path[];

extern const char* skybox_path[];

extern const char* mesh_path[];

void resources_initialize(Resources* resources);

void resources_load(Resources* resources);

void resources_unload(Resources* resources);

#endif
