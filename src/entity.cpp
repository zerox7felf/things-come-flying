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
	entity->position = entity->relative_pos = position;
	entity->size = size;
	entity->rotation = rotation;
	entity->rotation_pivot = rotation_pivot;
    entity->update = update;
	//entity->type = type;
	entity->move_speed = DEFAULT_ENTITY_MOVE_SPEED;
	entity->angular_speed = DEFAULT_ENTITY_ANGULAR_SPEED;
	entity->mesh_id = mesh_id;
	entity->parent = parent;
	entity->following = following;
	entity->animate_texture = 0;
	return entity;
}

mat4 entity_get_transform(Entity* entity) {
    mat4 model;

    if (entity->parent) {
	    model = multiply_mat4(entity_get_transform(entity->parent), translate(entity->position));
    } else if (entity->following) {
	    model = translate(entity->position);
    } else {
	    model = translate(entity->position);
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
	if (entity->animate_texture) {
		Material* material = &entity->material;
		material->texture1.offset = V2(engine->total_time * -0.025f, 0);
	}
}

void entity_render(Entity* entity, Scene* scene) {
	if (entity->mesh_id >= 0) {
		render_mesh(entity_get_transform(entity), entity->mesh_id, entity->material, scene);
	}
}
