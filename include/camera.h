// camera.h

#ifndef _CAMERA_H
#define _CAMERA_H

typedef struct Camera {
  v3 pos;
  v3 up;
  v3 right;
  v3 forward;
  double pitch;
  double yaw;
} Camera;

extern Camera camera;

void camera_initialize(v3 pos);

void camera_update();

#endif
