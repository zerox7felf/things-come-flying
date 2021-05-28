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
	entity_initialize(sun, V3(0, 0, 0), V3(3, 3, 3), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, NULL);
	sun->move_speed = 0;
	Material sun_material = base;
	sun_material.ambient.value.constant = 1.0f;
	sun_material.color_map.id= TEXTURE_SUN;
	entity_attach_material(sun, sun_material);

	Entity* earth = engine_push_empty_entity(engine);
	entity_initialize(earth, V3(15, 0, 14), V3(0.75f, 0.75f, 0.75f), V3(20, 0, 0), ENTITY_PLANET, MESH_SPHERE, sun);
	Material earth_material = base;
	earth_material.ambient.value.map.id = TEXTURE_EARTH_NIGHT;
	earth_material.ambient.type = VALUE_MAP_MAP;
	earth_material.specular.value.map.id = TEXTURE_EARTH_SPECULAR;
	earth_material.specular.type = VALUE_MAP_MAP;
	earth_material.color_map.id = TEXTURE_EARTH;
	earth_material.texture1 = { .id = TEXTURE_EARTH_CLOUDS, };	// TODO(lucas): Animate secondary texture in entities
	earth_material.texture_mix = 1.0f;
	entity_attach_material(earth, earth_material);

    // TODO: position parented entities in their parents coordinate system
    Entity* house = engine_push_empty_entity(engine);
    entity_initialize(house, V3(0, 0.8f, 0), V3(0.1f, 0.1f, 0.1f), V3(0, 0, 0), ENTITY_PLANET, MESH_HOUSE, earth);
    Material house_material = base;
    house_material.ambient.value.constant = 0.1f;
    house_material.specular.value.map.id = TEXTURE_HOUSE_SPECULAR;
    house_material.specular.type = VALUE_MAP_MAP;
    house_material.normal.value.map.id = TEXTURE_HOUSE_NORMAL;
    house_material.normal.type = VALUE_MAP_MAP;
    house_material.color_map.id = TEXTURE_HOUSE;
    entity_attach_material(house, house_material);

	Entity* moon = engine_push_empty_entity(engine);
	entity_initialize(moon, V3(2.2f, 0, 2), V3(0.25f, 0.25f, 0.25f), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, earth);
	moon->move_speed = DEFAULT_ENTITY_MOVE_SPEED * 1.45f;
	Material moon_material = base;
	moon_material.color_map.id = TEXTURE_MOON;
	entity_attach_material(moon, moon_material);

	Entity* alien = engine_push_empty_entity(engine);
	entity_initialize(alien, V3(35, 0, 30), V3(2, 2, 2), V3(0, 0, 0), ENTITY_PLANET, MESH_SPHERE, sun);
	alien->move_speed = DEFAULT_ENTITY_MOVE_SPEED * 0.35f;
	Material alien_material = base;
	alien_material.ambient.value.map.id = TEXTURE_ALIEN_AMBIENT;
	alien_material.ambient.type = VALUE_MAP_MAP;
	alien_material.color_map.id = TEXTURE_ALIEN;
	entity_attach_material(alien, alien_material);
}

i32 engine_run(Engine* engine) {
    float shine = 1.0f;
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
#if 0 	// NOTE(lucas): Camera zoom has these controls now ;)
        if (key_down[GLFW_KEY_UP]) {
            shine += 0.1;
            printf("Shine: %f\n", shine);
        }
        if (key_down[GLFW_KEY_DOWN]) {
            shine -= 0.1;
            printf("Shine: %f\n", shine);
        }
#endif
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
			}
			else if (key_pressed[GLFW_KEY_RIGHT]) {
				engine->target_entity_index = (engine->target_entity_index + 1) % engine->entity_count;
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

		renderer_bind_fbo(FBO_COLOR);

		render_skybox(CUBE_MAP_SPACE, 0.7f);

#if 0
		v3 alien_pos = V3(35 * cos(engine->total_time * 0.85f), 2 * cos(engine->total_time * 0.85f), 35 * sin(engine->total_time * 0.85f));
		v3 alien_size = V3(1.5f, 1.5f, 1.5f);

		v3 earth_pos = V3(20 * cos(engine->total_time), 0, 20 * sin(engine->total_time));
		v3 earth_size = alien_size * 0.5f;

		v3 moon_pos = earth_pos + V3(4 * cos(1.2f * engine->total_time), 0, 4 * sin(1.2f * engine->total_time));
		v3 moon_size = earth_size * 0.5f;

		v3 guy_pos = moon_pos + V3(1 * cos(2.5f * engine->total_time), 0, 1 * sin(2.5f * engine->total_time));
		v3 guy_size = moon_size * 0.2f;
#endif
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

		for (u32 entity_index = 0; entity_index < engine->entity_count; ++entity_index) {
			Entity* entity = &engine->entities[entity_index];
			entity_update(entity, engine);
			entity_render(entity);
		}

#if 0
        render_mesh(earth_pos, V3(20, angle, 0), earth_size, MESH_SPHERE, (Material) {
            .ambient = {
                .value = { .constant = fullbright ? 1.0f : 0.05f },
                //.value = { .map = { .id = TEXTURE_EARTH_AMBIENT } },
                .type = VALUE_MAP_CONST,
            },
            .diffuse = {
                .value = { .constant = 1.0f },
                .type = VALUE_MAP_CONST
            },
            .specular = {
                .value = { .map = { .id = TEXTURE_EARTH_SPECULAR } },
                .type = VALUE_MAP_MAP
            },
            .shininess = shine,
            .color_map = {.id = TEXTURE_EARTH},
			.texture1 = {.id = TEXTURE_EARTH_CLOUDS, .offset = V2(-0.05f * engine->total_time, 0)},
			.texture_mix = 1.0f,
        });

		render_mesh(alien_pos, V3(-25, angle * 1.25f, 0), alien_size, MESH_SPHERE, (Material) {
            .ambient = {
                .value = { .map = { .id = TEXTURE_ALIEN_AMBIENT } },
                .type = VALUE_MAP_MAP
            },
            .diffuse = {
                .value = { .constant = 1.0f },
                .type = VALUE_MAP_CONST
            },
            .specular = {
                .value = { .constant = 0.5f },
                .type = VALUE_MAP_CONST
            },
            .shininess = 10.0f,
            .color_map = {.id = TEXTURE_ALIEN},
			.texture1 = {},
			.texture_mix = 0,
        });

		render_mesh(V3(0, 0, 0), V3(0, angle, 0), V3(4, 4, 4), MESH_SPHERE, (Material) {
            .ambient = {
                .value = { .constant = 1.0f },
                .type = VALUE_MAP_CONST
            },
            .diffuse = {
                .value = { .constant = 1.0f },
                .type = VALUE_MAP_CONST
            },
            .specular = {
                .value = { .constant = 0.5f },
                .type = VALUE_MAP_CONST
            },
            .shininess = 10.0f,
            .color_map = {.id = TEXTURE_SUN},
			.texture1 = {},
			.texture_mix = 0,
        });
#endif

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
	}
	assert("memory leak" && (memory_total_allocated() == 0));
	return result;
}
