// engine.cpp

#include <sys/time.h>	// gettimeofday

#include "engine.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "renderer.hpp"

#define MAX_DT 1.0f
#define TITLE_SIZE 128

Engine engine;

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
	camera_initialize(V3(0, 0, -14));
}

i32 engine_run(Engine* engine) {
	float angle = 0.0f;
    float shine = 1.0f;
	struct timeval now = {0};
	struct timeval prev = {0};
	u8 follow_target = 0;
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
        if (key_down[GLFW_KEY_UP]) {
            shine += 0.1;
            printf("Shine: %f\n", shine);
        }
        if (key_down[GLFW_KEY_DOWN]) {
            shine -= 0.1;
            printf("Shine: %f\n", shine);
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
		if (key_pressed[GLFW_KEY_F]) {
			follow_target = !follow_target;
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

		window_get_cursor(&engine->mouse_x, &engine->mouse_y);
		camera_update();

		renderer_bind_fbo();
		renderer_clear_fbo();

		render_skybox(CUBE_MAP_SPACE, 0.7f);

		v3 alien_pos = V3(35 * cos(engine->total_time * 0.85f), 2 * cos(engine->total_time * 0.85f), 35 * sin(engine->total_time * 0.85f));
		v3 alien_size = V3(1.5f, 1.5f, 1.5f);

		v3 earth_pos = V3(20 * cos(engine->total_time), 0, 20 * sin(engine->total_time));
		v3 earth_size = alien_size * 0.5f;

		v3 moon_pos = earth_pos + V3(4 * cos(1.2f * engine->total_time), 0, 4 * sin(1.2f * engine->total_time));
		v3 moon_size = earth_size * 0.5f;

		v3 guy_pos = moon_pos + V3(1 * cos(2.5f * engine->total_time), 0, 1 * sin(2.5f * engine->total_time));
		v3 guy_size = moon_size * 0.2f;

		if (follow_target) {
			camera.target_pos = earth_pos - camera.forward * 1.5f;
		}

        render_mesh(earth_pos, V3(20, angle, 0), earth_size, MESH_SPHERE, (Material) {
            .ambient = fullbright ? 1.0f : 0.05f,
            .diffuse = 1.0f,
            .specular = 0.5f,
            .shininess = shine,
            .texture0 = {.id = TEXTURE_EARTH},
			.texture1 = {.id = TEXTURE_EARTH_CLOUDS, .offset = V2(-0.05f * engine->total_time, 0)},
			.texture_mix = 1.0f,
        });

        render_mesh(V3(0, 0, 0), V3(0, angle, 0), V3(3, 3, 3), MESH_SPHERE, (Material) {
            .ambient = fullbright ? 1.0f : 1.0f,
            .diffuse = 0.0f,
            .specular = 0.5f,
            .shininess = 10.0f,
            .texture0 = {.id = TEXTURE_SUN},
			.texture1 = {},
			.texture_mix = 0,
        });

		render_mesh(alien_pos, V3(-25, angle * 1.25f, 0), alien_size, MESH_SPHERE, (Material) {
            .ambient = fullbright ? 1.0f : 0.01f,
            .diffuse = 1.0f,
            .specular = 0.5f,
            .shininess = 10.0f,
            .texture0 = {.id = TEXTURE_ALIEN},
			.texture1 = {},
			.texture_mix = 0,
        });

		render_mesh(moon_pos, V3(0, 0, 0), moon_size, MESH_SPHERE, (Material) {
            .ambient = fullbright ? 1.0f : 0.01f,
            .diffuse = 1.0f,
            .specular = 0.5f,
            .shininess = 10.0f,
			.texture0 = {.id = TEXTURE_MOON},
			.texture1 = {},
			.texture_mix = 0,
        });

		render_mesh(guy_pos, V3(angle * 2, angle * 1.2f, -angle), guy_size, MESH_GUY, (Material) {
            .ambient = fullbright ? 1.0f : 0.02f,
            .diffuse = 1.0f,
            .specular = 0.5f,
            .shininess = 1.0f,
            .texture0 = {.id = TEXTURE_GUY},
			.texture1 = {},
			.texture_mix = 0,
        });

        render_mesh(V3(20.0f, 6.0f, 20.0f), V3(angle * 6, angle * 3.6f, angle * -2.0f), V3(1.0f, 1.0f, 1.0f), MESH_CUBE, (Material) {
            .ambient = fullbright ? 1.0f : 0.1f,
            .diffuse = 1.0f,
            .specular = 0.5f,
            .shininess = 10.0f,
            .texture0 = {.id = TEXTURE_MOON},
			.texture1 = {},
			.texture_mix = 0,
        });

		angle = 10 * engine->total_time;

		renderer_unbind_fbo();

		render_fbo();

		snprintf(title_string, TITLE_SIZE, "Solar System | %i fps | %g delta", (i32)(1.0f / engine->delta_time), engine->delta_time);

		window_set_title(title_string);

		window_swap_buffers();
		window_clear_buffers(0, 0, 0);
	}
	return NoError;
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
