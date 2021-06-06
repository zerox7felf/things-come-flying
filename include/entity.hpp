// entity.hpp

#ifndef _ENTITY_HPP
#define _ENTITY_HPP

#include "renderer.hpp"

#define DEFAULT_ENTITY_ANGULAR_SPEED 7.5f
#define DEFAULT_ENTITY_MOVE_SPEED 0.5f

typedef struct Entity {
	v3 world_position;
	v3 position;
	v3 relative_pos;
	v3 size;
	v3 rotation;
    v3 rotation_pivot;
    void (*update)(Entity* entity, struct Engine* engine);
	float move_speed;
	float angular_speed;
	i32 mesh_id;
	struct Entity* parent;      // Adopt parent origin and coordinate system
	struct Entity* following;   // Simply follow
    // NOTE(linus): are the two fields above fine or do we want another solution (eg. per-property parenting or smthn)?

	Material material;
} Entity;

typedef void (*Entity_update)(Entity* entity, struct Engine* engine);

Entity* entity_initialize(
    Entity* entity,
    v3 position,
    v3 size,
    v3 rotation,
    v3 rotation_pivot,
    //void (*update)(Entity* entity, struct Engine* engine),
    Entity_update update,
    i32 mesh_id,
    Entity* parent,
    Entity* following
);

mat4 entity_get_transform(Entity* entity);

void entity_update(Entity* entity, Engine* engine);

void entity_render(Entity* entity, Scene* scene);

#endif
