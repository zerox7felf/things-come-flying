// entity.cpp

#include "common.hpp"
#include "engine.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "matrix_math.hpp"

Entity* entity_initialize(
    Entity* entity,
    v3 position,
    v3 size,
    v3 rotation,
    v3 rotation_pivot,
    void (*update)(Entity* entity, struct Engine* engine),
    i32 mesh_id,
    Entity* parent,
    Entity* following
){
	memset(entity, 0, sizeof(Entity));
	entity->world_position = entity->position = entity->relative_pos = position;
	entity->size = size;
	entity->rotation = rotation;
	entity->rotation_pivot = rotation_pivot;
    entity->update = update;
	// entity->type = type;
	entity->move_speed = DEFAULT_ENTITY_MOVE_SPEED;
	entity->angular_speed = DEFAULT_ENTITY_ANGULAR_SPEED;
	entity->mesh_id = mesh_id;
	entity->parent = parent;
	entity->following = following;
	return entity;
}

mat4 entity_get_transform(Entity* entity) {
    mat4 model;

    if (entity->parent) {
	    model = multiply_mat4(entity_get_transform(entity->parent), translate(entity->position));
		entity->world_position = multiply_mat4_v3(model, V3(0, 0, 0));
    } else if (entity->following) {
	    model = translate(entity->position);
		entity->world_position = entity->position;
    } else {
	    model = translate(entity->position);
		entity->world_position = entity->position;
    }

    model = multiply_mat4(model, translate(entity->rotation_pivot));
	model = multiply_mat4(model, rotate(entity->rotation.y, V3(0.0f, 1.0f, 0.0f)));
	model = multiply_mat4(model, rotate(entity->rotation.z, V3(0.0f, 0.0f, 1.0f)));
	model = multiply_mat4(model, rotate(entity->rotation.x, V3(1.0f, 0.0f, 0.0f)));
    model = multiply_mat4(model, translate(entity->rotation_pivot * -1.0f));

	model = multiply_mat4(model, scale_mat4(entity->size));

    return model;
}

void entity_update(Entity* entity, Engine* engine) {
}

void entity_render(Entity* entity, Scene* scene) {
	if (entity->mesh_id >= 0) {
		render_mesh(entity_get_transform(entity), entity->mesh_id, entity->material, scene);
	}
}
