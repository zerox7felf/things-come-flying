// resource.hpp

#ifndef _RESOURCE_HPP
#define _RESOURCE_HPP

#include "common.hpp"
#include "mesh.hpp"
#include "image.hpp"

enum Texture_id {
	TEXTURE_MISSING = 0,
	TEXTURE_EARTH,
	TEXTURE_EARTH_CLOUDS,
	TEXTURE_EARTH_AMBIENT,
	TEXTURE_EARTH_SPECULAR,
	TEXTURE_EARTH_NIGHT,
	TEXTURE_SUN,
	TEXTURE_ALIEN,
	TEXTURE_ALIEN_AMBIENT,
	TEXTURE_ALIEN_SPECULAR,
	TEXTURE_MOON,
    TEXTURE_SHINGLES,
    TEXTURE_SHINGLES_SPECULAR,
    TEXTURE_SHINGLES_NORMAL,
    TEXTURE_HOUSE,
    TEXTURE_HOUSE_SPECULAR,
    TEXTURE_HOUSE_NORMAL,
	TEXTURE_GUY,
	TEXTURE_GREEN,
	TEXTURE_WHITE,

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
    MESH_MONKE_FLAT,
    MESH_PLANE,
    MESH_BENT_PLANE,
    MESH_HOUSE,
    MESH_QUAD,

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
