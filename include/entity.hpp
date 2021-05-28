// entity.hpp

#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include "renderer.hpp"

#define DEFAULT_ENTITY_ANGULAR_SPEED 7.5f
#define DEFAULT_ENTITY_MOVE_SPEED 0.5f

typedef enum Entity_type {
	ENTITY_NONE = 0,
	ENTITY_PLANET,
	ENTITY_CAMERA_ATTACHER,
} Entity_type;

typedef struct Entity {
	v3 position;
	v3 relative_pos;
	v3 size;
	v3 rotation;
	Entity_type type;
	float move_speed;
	float angular_speed;
	i32 mesh_id;
	struct Entity* parent;

	Material material;
} Entity;

Entity* entity_initialize(Entity* entity, v3 position, v3 size, v3 rotation, Entity_type type, i32 mesh_id, Entity* parent);

void entity_attach_material(Entity* entity, Material material);

void entity_update(Entity* entity, struct Engine* engine);

void entity_render(Entity* entity);

#endif
