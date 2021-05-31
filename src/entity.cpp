// entity.cpp

#include "common.hpp"
#include "engine.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "matrix_math.hpp"

Entity* entity_initialize(Entity* entity, v3 position, v3 size, v3 rotation, v3 rotation_pivot, Entity_type type, i32 mesh_id, Entity* parent, Entity* following) {
	memset(entity, 0, sizeof(Entity));
	entity->position = entity->relative_pos = position;
	entity->size = size;
	entity->rotation = rotation;
	entity->rotation_pivot = rotation_pivot;
	entity->type = type;
	entity->move_speed = DEFAULT_ENTITY_MOVE_SPEED;
	entity->angular_speed = DEFAULT_ENTITY_ANGULAR_SPEED;
	entity->mesh_id = mesh_id;
	entity->parent = parent;
	entity->following = following;
	return entity;
}

void entity_attach_material(Entity* entity, Material material) {
	entity->material = material;
}

v3 entity_get_worldspace_pos(Entity* entity) {
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
	switch (entity->type) {
		case ENTITY_PLANET: {
			v3 following_position = entity->following ? entity->following->position : V3(0, 0, 0);
            float distance_factor = 20.0f / (length_v3(entity->relative_pos) + 1);
            float spin_time = engine->total_time * entity->move_speed * distance_factor;
			entity->position = following_position + V3(
                entity->relative_pos.x * cos(spin_time),
                entity->relative_pos.y,
                entity->relative_pos.z * sin(spin_time)
            );
			entity->rotation = V3(
                entity->rotation.x,
                fmodf(engine->total_time * entity->angular_speed, 360),
                entity->rotation.x
            );
			break;
		}
		case ENTITY_CAMERA_ATTACHER: {
			break;
		}
        case ENTITY_SPIN: {
			entity->rotation = V3(
                entity->rotation.x,
                fmodf(engine->total_time * entity->angular_speed, 360),
                entity->rotation.x
            );
        };
		default:
			break;
	}
}

void entity_render(Entity* entity, Scene* scene) {
	if (entity->mesh_id >= 0) {
		render_mesh(entity_get_transform(entity), entity->mesh_id, entity->material, scene);
	}
}
