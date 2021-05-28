// entity.cpp

#include "common.hpp"
#include "engine.hpp"
#include "camera.hpp"
#include "entity.hpp"

Entity* entity_initialize(Entity* entity, v3 position, v3 size, v3 rotation, Entity_type type, i32 mesh_id, Entity* parent) {
	memset(entity, 0, sizeof(Entity));
	entity->position = entity->relative_pos = position;
	entity->size = size;
	entity->rotation = rotation;
	entity->type = type;
	entity->move_speed = DEFAULT_ENTITY_MOVE_SPEED;
	entity->angular_speed = DEFAULT_ENTITY_ANGULAR_SPEED;
	entity->mesh_id = mesh_id;
	entity->parent = parent;
	return entity;
}

void entity_attach_material(Entity* entity, Material material) {
	entity->material = material;
}

void entity_update(Entity* entity, Engine* engine) {
	switch (entity->type) {
		case ENTITY_PLANET: {
			const v2 distance = V2(20, 20);
			v3 parent_position = V3(0, 0, 0);
			if (entity->parent) {
				parent_position = entity->parent->position;
			}
			else {
				entity->relative_pos = entity->position;
			}
			entity->position = parent_position + V3(entity->relative_pos.x * cos(engine->total_time * entity->move_speed), entity->relative_pos.y, entity->relative_pos.z * sin(engine->total_time * entity->move_speed));
			entity->rotation = V3(entity->rotation.x, fmodf(engine->total_time * entity->angular_speed, 360), entity->rotation.x);
			break;
		}
		case ENTITY_CAMERA_ATTACHER: {
			break;
		}
		default:
			break;
	}
}

void entity_render(Entity* entity) {
	if (entity->mesh_id >= 0) {
		render_mesh(entity->position, entity->rotation, entity->size, entity->mesh_id, entity->material);
	}
}
