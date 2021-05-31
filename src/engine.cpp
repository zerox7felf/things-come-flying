// engine.cpp

#include <sys/time.h>	// gettimeofday

#include "window.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include "engine.hpp"

#define MAX_DT 1.0f
#define TITLE_SIZE 128

Engine engine = {};
i32 monkey_light_index = 0;
Entity* monkey = NULL;

static void engine_initialize(Engine* engine);
static i32 engine_run(Engine* engine);

// Entity* entity_initialize(Entity* entity, v3 position, v3 size, v3 rotation, Entity_type type, i32 mesh_id, Entity* parent);
void engine_initialize(Engine* engine) {
	engine->is_running = 1;
	engine->animation_playing = 1;
	engine->delta_time = 0;
	engine->total_time = 0;
	engine->time_scale = 1.0f;
	engine->mouse_x = 0;
	engine->mouse_y = 0;
	engine->scroll_x = 0;
	engine->scroll_y = 0;
	engine->entity_count = 0;
	engine->target_entity_index = 0;
	camera_initialize(V3(0, 0, -14));

    Point_light* lights = NULL;
    i32 num_lights = 0;
    /*Point_light sun_light = (Point_light) {
        .position = V3(0, 0, 0),
        .color = V3(1, 1, 1),
        .ambient = 1.0f,
        .falloff_linear = 0, //0.022f,
        .falloff_quadratic = 0 //0.0019f,
    };
    list_push(lights, num_lights, sun_light);*/

    Point_light monkey_light = (Point_light) {
        .position = V3(0, 0, 0),
        .color = V3(1, 1, 1),
        .ambient = 1.0f,
        .falloff_linear = 0.022f,
        .falloff_quadratic = 0.0019f
    };
    monkey_light_index = num_lights;
    list_push(lights, num_lights, monkey_light);

    engine->scene = (Scene) {
        .lights = lights,
        .num_lights = num_lights
    };

	Material base = (Material) {
		.ambient = {
			.value = { .constant = 0.02f, },
			.type = VALUE_MAP_CONST,
		},
		.diffuse = {
			.value = { .constant = 1.0f, },
			.type = VALUE_MAP_CONST,
		},
		.specular = {
			.value = { .constant = 1.0f, },
			.type = VALUE_MAP_CONST,
		},
        .normal = {
            .value = { .constant = 1.0f }, // NOTE(linus): Constants here be ignored by shader, but they remain to keep normal maps optional.
            .type = VALUE_MAP_CONST
        },
		.shininess = 10.0f,	// NOTE(lucas): Looks a bit strange in lower shininess values
                            // NOTE(linus): ~10 should be a better default
		.color_map = { .id = TEXTURE_WHITE, },
		.texture1 = {},
		.texture_mix = 0,
	};

	Entity* sun = engine_push_empty_entity(engine);
	entity_initialize(sun, V3(0, 0, 0), V3(4, 4, 4), V3(0, 0, 0), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, NULL, NULL);
	sun->move_speed = 0;
	Material sun_material = base;
	sun_material.ambient.value.constant = 1.1f;
	sun_material.color_map.id= TEXTURE_SUN;
	entity_attach_material(sun, sun_material);

	Entity* earth = engine_push_empty_entity(engine);
	entity_initialize(earth, V3(50, 0, 50), V3(0.75f, 0.75f, 0.75f), V3(20, 0, 0), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, NULL, sun);
	Material earth_material = base;
	earth_material.ambient.value.map.id = TEXTURE_EARTH_NIGHT;
	earth_material.ambient.type = VALUE_MAP_MAP;
	earth_material.specular.value.map.id = TEXTURE_EARTH_SPECULAR;
	earth_material.specular.type = VALUE_MAP_MAP;
	earth_material.color_map.id = TEXTURE_EARTH;
	earth_material.texture1 = { .id = TEXTURE_EARTH_CLOUDS, };	// TODO(lucas): Animate secondary texture in entities
	earth_material.texture_mix = 1.0f;
	entity_attach_material(earth, earth_material);

    Entity* house = engine_push_empty_entity(engine);
    entity_initialize(house, V3(0, 1.05f, 0), V3(.1f,.1f,.1f), V3(0, 0, -75), V3(0, -1.05f, 0), ENTITY_NONE, MESH_HOUSE, earth, NULL);
    Material house_material = base;
    house_material.ambient.value.constant = 0.1f;
    house_material.specular.value.map.id = TEXTURE_HOUSE_SPECULAR;
    house_material.specular.type = VALUE_MAP_MAP;
    house_material.normal.value.map.id = TEXTURE_HOUSE_NORMAL;
    house_material.normal.type = VALUE_MAP_MAP;
    house_material.color_map.id = TEXTURE_HOUSE;
    entity_attach_material(house, house_material);

	Entity* moon = engine_push_empty_entity(engine);
	entity_initialize(moon, V3(2.2f, 0, 2), V3(0.25f, 0.25f, 0.25f), V3(0, 0, 0), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, NULL, earth);
	moon->move_speed = DEFAULT_ENTITY_MOVE_SPEED * 1.45f;
	Material moon_material = base;
	moon_material.color_map.id = TEXTURE_MOON;
	entity_attach_material(moon, moon_material);

	Entity* alien = engine_push_empty_entity(engine);
	entity_initialize(alien, V3(35, 0, 30), V3(2, 2, 2), V3(0, 0, 0), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, NULL, sun);
	alien->move_speed = DEFAULT_ENTITY_MOVE_SPEED * 0.35f;
	Material alien_material = base;
	alien_material.ambient.value.map.id = TEXTURE_ALIEN_AMBIENT;
	alien_material.ambient.type = VALUE_MAP_MAP;
	alien_material.color_map.id = TEXTURE_ALIEN;
	entity_attach_material(alien, alien_material);

    monkey = engine_push_empty_entity(engine);
    entity_initialize(monkey, V3(32, 0, 33.18f), V3(2.5f, 2.5f, 2.5f), V3(0, 0, 0), V3(0, 0, 0), ENTITY_NONE, MESH_MONKE, NULL, NULL);
    Material monkey_material = base;
    monkey_material.color_map.id = TEXTURE_GREEN;
    monkey_material.ambient.value.constant = 1.0f;
	entity_attach_material(monkey, monkey_material);

    Entity* floor = engine_push_empty_entity(engine);
    entity_initialize(floor, V3(0, -20.0f, 0), V3(10, 10, 10), V3(0, 0, 0), V3(0, 0, 0), ENTITY_NONE, MESH_PLANE, NULL, NULL);
    Material floor_material = base;
    floor_material.color_map.id = TEXTURE_SHINGLES;
    floor_material.normal.value.map.id = TEXTURE_SHINGLES_NORMAL;
    floor_material.normal.type = VALUE_MAP_MAP;
    entity_attach_material(floor, floor_material);
}

i32 engine_run(Engine* engine) {
	struct timeval now = {};
	struct timeval prev = {};
	u8 follow_target = 1;
	Entity* target_entity = NULL;
	char title_string[TITLE_SIZE] = {0};
    u8 fullbright = 0;
	while (engine->is_running && window_poll_events() >= 0) {
		prev = now;
		gettimeofday(&now, NULL);
		engine->delta_time = ((((now.tv_sec - prev.tv_sec) * 1000000.0f) + now.tv_usec) - (prev.tv_usec)) / 1000000.0f;
		if (engine->delta_time >= MAX_DT) {
			engine->delta_time = MAX_DT;
		}
		if (engine->animation_playing) {
			engine->total_time += engine->delta_time * engine->time_scale;
		}
		if (key_pressed[GLFW_KEY_SPACE]) {
			engine->animation_playing = !engine->animation_playing;
		}
		if (key_pressed[GLFW_KEY_ESCAPE]) {
			engine->is_running = 0;
		}
		if (key_pressed[GLFW_KEY_F11]) {
			window_toggle_fullscreen();
		}
		if (key_pressed[GLFW_KEY_1]) {
			if (engine->time_scale < 0.1f) {
				engine->time_scale *= 0.5f;
			}
			else {
				engine->time_scale -= 0.05f;
			}
			fprintf(stdout, "Time scale: %g\n", engine->time_scale);
		}
		if (key_pressed[GLFW_KEY_2]) {
			engine->time_scale += 0.05f;
			fprintf(stdout, "Time scale: %g\n", engine->time_scale);
		}
		if (key_pressed[GLFW_KEY_3]) {
			engine->time_scale = 1;
			fprintf(stdout, "Reset time scale: %g\n", engine->time_scale);
		}
		if (key_pressed[GLFW_KEY_R]) {
			engine_initialize(engine);
			continue;
		}
		if (key_pressed[GLFW_KEY_M]) {
			window_toggle_cursor_visibility();
		}
        if (key_pressed[GLFW_KEY_B]) {
            fullbright = !fullbright;
        }
        if (key_pressed[GLFW_KEY_P]) {
			renderer_toggle_post_processing();
        }
		if (key_pressed[GLFW_KEY_I]) {
			camera.interactive_mode = !camera.interactive_mode;
		}
		if (follow_target) {
			if (key_pressed[GLFW_KEY_LEFT]) {
				if (engine->target_entity_index > 0) {
					engine->target_entity_index--;
				}
				else {
					engine->target_entity_index = engine->entity_count - 1;
				}
                printf("Now following %d.\n", engine->target_entity_index);
			}
			else if (key_pressed[GLFW_KEY_RIGHT]) {
				engine->target_entity_index = (engine->target_entity_index + 1) % engine->entity_count;
                printf("Now following %d.\n", engine->target_entity_index);
			}
			else if (key_down[GLFW_KEY_UP]) {
				camera.zoom_target -= 10 * engine->delta_time;
				camera.zoom_target = clamp(camera.zoom_target, 1.0f, 255.0f);
			}
			else if (key_down[GLFW_KEY_DOWN]) {
				camera.zoom_target += 10 * engine->delta_time;
				camera.zoom_target = clamp(camera.zoom_target, 1.0f, 255.0f);
			}
			target_entity = engine_get_target(engine);
		}
		if (key_pressed[GLFW_KEY_F]) {
			follow_target = !follow_target;
			if (follow_target) {
				target_entity = engine_get_target(engine);
			}
			else {
				target_entity = NULL;
			}
		}
        if (key_pressed[GLFW_KEY_KP_8]) {
            //monkey->position.x += 1.0f;
            engine->scene.lights[monkey_light_index].position.x += 1.0f;
        }
        if (key_pressed[GLFW_KEY_KP_2]) {
            //monkey->position.x -= 1.0f;
            engine->scene.lights[monkey_light_index].position.x -= 1.0f;
        }
        if (key_pressed[GLFW_KEY_KP_4]) {
            //monkey->position.z += 1.0f;
            engine->scene.lights[monkey_light_index].position.z += 1.0f;
        }
        if (key_pressed[GLFW_KEY_KP_6]) {
            //monkey->position.z -= 1.0f;
            engine->scene.lights[monkey_light_index].position.z -= 1.0f;
        }
        if (key_pressed[GLFW_KEY_KP_7]) {
            //monkey->position.y += 1.0f;
            engine->scene.lights[monkey_light_index].position.y += 1.0f;
        }
        if (key_pressed[GLFW_KEY_KP_1]) {
            //monkey->position.y -= 1.0f;
            engine->scene.lights[monkey_light_index].position.y -= 1.0f;
        }

		renderer_bind_fbo(FBO_COLOR);

		render_skybox(CUBE_MAP_SPACE, 0.2f);

        //engine->scene.lights[monkey_light_index].position = monkey->position;
        printf(
            "Light:\nx:%f,\ny:%f,\nz:%f\n---------\n",
            engine->scene.lights[monkey_light_index].position.x,
            engine->scene.lights[monkey_light_index].position.y,
            engine->scene.lights[monkey_light_index].position.z
        );
		for (u32 entity_index = 0; entity_index < engine->entity_count; ++entity_index) {
			Entity* entity = &engine->entities[entity_index];
			entity_update(entity, engine);
			entity_render(entity, &engine->scene);
		}

		if (engine->scroll_y != 0) {
			camera.zoom_target -= 0.1f * engine->scroll_y;
			camera.zoom_target = clamp(camera.zoom_target, 1.0f, 255.0f);
		}

		if (follow_target && target_entity) {
			camera.interpolate = 0;
			camera.target_pos = target_entity->position - camera.forward * camera.zoom;
		}
		else {
			camera.interpolate = 1;
		}

		window_get_cursor(&engine->mouse_x, &engine->mouse_y);
		window_get_scroll(&engine->scroll_x, &engine->scroll_y);
		camera_update(engine);

		snprintf(title_string, TITLE_SIZE, "Solar System | %i fps | %g delta", (i32)(1.0f / engine->delta_time), engine->delta_time);
		window_set_title(title_string);

		renderer_post_process();
		window_swap_buffers();
		renderer_clear_fbos();
	}
	return NoError;
}

Entity* engine_push_empty_entity(Engine* engine) {
	Entity* e = NULL;
	if (engine->entity_count < MAX_ENTITY) {
		e = &engine->entities[engine->entity_count++];
		memset(e, 0, sizeof(Entity));
		return e;
	}
	return NULL;
}

Entity* engine_get_target(Engine* engine) {
	if (engine->target_entity_index < engine->entity_count) {
		return &engine->entities[engine->target_entity_index];
	}
	return NULL;
}

i32 engine_start() {
	i32 result = NoError;
	engine_initialize(&engine);

	if ((result = window_open("Solar System", 800, 600, 0 /* fullscreen */, 0 /* vsync */, renderer_framebuffer_callback)) == NoError) {
		renderer_initialize();
		engine_run(&engine);
		window_close();
		renderer_destroy();
        list_free(engine.scene.lights, engine.scene.num_lights);
	}
	assert("memory leak" && (memory_total_allocated() == 0));
	return result;
}
