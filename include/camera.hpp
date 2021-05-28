// camera.hpp

#ifndef _CAMERA_HPP
#define _CAMERA_HPP

typedef struct Camera {
	v3 pos;
	v3 target_pos;
	v3 up;
	v3 right;
	v3 forward;
	float pitch;
	float yaw;
	float zoom;
	float zoom_target;
	u8 interpolate;
	u8 interactive_mode;
} Camera;

extern Camera camera;

void camera_initialize(v3 pos);

void camera_update(struct Engine* engine);

#endif
