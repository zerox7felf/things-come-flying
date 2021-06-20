// resource.hpp

#ifndef _RESOURCE_HPP
#define _RESOURCE_HPP

#include "common.hpp"
#include "mesh.hpp"
#include "image.hpp"

enum Shader_id {
    DIFFUSE_SHADER = 0,
	SKYBOX_SHADER,
	TEXTURE_SHADER,
	COMBINE_SHADER,
	BLUR_SHADER,
    FLARE_SHADER,
	BRIGHTNESS_EXTRACT_SHADER,
    GROUND_SHADER,
    MAX_SHADER
};

enum Texture_id {
	TEXTURE_MISSING = 0,
    TEXTURE_HOUSE,
    TEXTURE_HOUSE_SPECULAR,
    TEXTURE_HOUSE_NORMAL,
    TEXTURE_GROUND01,

    TEXTURE_LENSFLARE_1,
    TEXTURE_LENSFLARE_2,
    TEXTURE_LENSFLARE_3,

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
    MESH_DESTROYER,
    MESH_SATURN_RINGS,
    MESH_GROUND01,
    MESH_GROUND01_WATER,

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

extern const char* shader_path[];

void resources_initialize(Resources* resources);

void resources_load(Resources* resources);

void resources_unload(Resources* resources);

#endif
