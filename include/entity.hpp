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
    v3 rotation_pivot;
	Entity_type type;
	float move_speed;
	float angular_speed;
	i32 mesh_id;
	struct Entity* parent;      // Adopt parent origin and coordinate system
	struct Entity* following;   // Simply follow
    // NOTE(linus): are the two fields above fine or do we want another solution (eg. per-property parenting or smthn)?

	Material material;
} Entity;

Entity* entity_initialize(Entity* entity, v3 position, v3 size, v3 rotation, v3 rotation_pivot, Entity_type type, i32 mesh_id, Entity* parent, Entity* following);

void entity_attach_material(Entity* entity, Material material);

mat4 entity_get_transform(Entity* entity);

void entity_update(Entity* entity, struct Engine* engine);

void entity_render(Entity* entity);

#endif
