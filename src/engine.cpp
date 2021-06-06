// engine.cpp

#include <sys/time.h>	// gettimeofday

#include "window.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "renderer.hpp"
#include "engine.hpp"
#include "scene.hpp"

#define MAX_DT 1.0f
#define TITLE_SIZE 128

Engine engine = {};
u8 free_mouse = 0;

static void engine_initialize(Engine* engine);
static i32 engine_run(Engine* engine);

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
	camera_initialize(V3(0, 0, -10));
}

i32 engine_run(Engine* engine) {
    if (!initialize_scene(engine, "01.scene")) {
        return Error;
    }
    Entity* ground = engine_push_empty_entity(engine);
    entity_initialize(
        ground,
        V3(0,-5,0), V3(1,1,1),
        V3(0,0,0), V3(0,0,0),
        NULL,
        MESH_PLANE,
        NULL, NULL
    );
    ground->material = (Material){
        .ambient    = {.value = {.constant = 0.5f}, .type = VALUE_MAP_CONST},
        .diffuse    = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .specular   = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .normal     = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .shininess  = 10.0f,
        .color_map  = {.id = TEXTURE_HOUSE},
        .texture1   = {}, .texture_mix = 0,
        .shader_index = DIFFUSE_SHADER
    };

    Entity* house = engine_push_empty_entity(engine);
    entity_initialize(
        house,
        V3(0,-5,0), V3(1,1,1),
        V3(0,0,0), V3(0,0,0),
        NULL,
        MESH_HOUSE,
        NULL, NULL
    );
    house->material = (Material){
        .ambient    = {.value = {.constant = 0.5f}, .type = VALUE_MAP_CONST},
        .diffuse    = {.value = {.constant = 1.0f}, .type = VALUE_MAP_CONST},
        .specular   = {.value = {.map = {.id = TEXTURE_HOUSE_SPECULAR}}, .type = VALUE_MAP_MAP},
        .normal     = {.value = {.map = {.id = TEXTURE_HOUSE_NORMAL}}, .type = VALUE_MAP_MAP},
        .shininess  = 10.0f,
        .color_map  = {.id = TEXTURE_HOUSE},
        .texture1   = {}, .texture_mix = 0,
        .shader_index = DIFFUSE_SHADER
    };

    Sun_light* lights = NULL;
    i32 num_lights = 0;
    Sun_light sun_light = (Sun_light) {
        .angle = V3(1, 0, 0),
        .color = V3(1, 1, 1),
        .ambient = 1.0f,
        .falloff_linear = 0,
        .falloff_quadratic = 0
    };
    list_push(lights, num_lights, sun_light);

    engine->scene = (Scene) {
        .lights = NULL,
        .num_lights = 0,
        .sun_lights = lights,
        .num_sun_lights = num_lights
    };

	struct timeval now = {};
	struct timeval prev = {};

	char title_string[TITLE_SIZE] = {0};
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
			//engine_initialize(engine);
			//continue;
            return 2;
		}
		if (key_pressed[GLFW_KEY_M]) {
			window_toggle_cursor_visibility();
			free_mouse = !free_mouse;
		}
        if (key_pressed[GLFW_KEY_P]) {
			renderer_toggle_post_processing();
        }
		if (key_pressed[GLFW_KEY_I]) {
			camera.interactive_mode = !camera.interactive_mode;
		}

		renderer_bind_fbo(FBO_COLOR);

		render_skybox(CUBE_MAP_SPACE, 1.0f);

		for (u32 entity_index = 0; entity_index < engine->entity_count; ++entity_index) {
			Entity* entity = &engine->entities[entity_index];
            if (entity->update != NULL)
                entity->update(entity, engine);
			entity_update(entity, engine);
			entity_render(entity, &engine->scene);
		}

		if (engine->scroll_y != 0) {
			camera.zoom_target -= 0.1f * engine->scroll_y;
			camera.zoom_target = clamp(camera.zoom_target, 1.0f, 255.0f);
		}

		if (!free_mouse) {
			window_get_cursor(&engine->mouse_x, &engine->mouse_y);
			window_get_scroll(&engine->scroll_x, &engine->scroll_y);
		}
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

i32 engine_start() {
	i32 result = NoError;
	engine_initialize(&engine);

	if ((result = window_open("Solar System", 800, 600, 0 /* fullscreen */, 0 /* vsync */, renderer_framebuffer_callback)) == NoError) {
        renderer_initialize();
        engine_initialize(&engine);
        while (engine_run(&engine) == 2) {
            list_free(engine.scene.lights, engine.scene.num_lights);
            list_free(engine.scene.sun_lights, engine.scene.num_sun_lights);
            engine_initialize(&engine);
        }
        list_free(engine.scene.lights, engine.scene.num_lights);
        list_free(engine.scene.sun_lights, engine.scene.num_sun_lights);
		window_close();
		renderer_destroy();
	}
	assert("memory leak" && (memory_total_allocated() == 0));
	return result;
}
