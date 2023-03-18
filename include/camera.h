#ifndef WHEN_THE_FACTORY_CAMERA_
#define WHEN_THE_FACTORY_CAMERA_
#include <SDL2/SDL.h>

#include "map.h"

struct Camera {
	SDL_FPoint target_pos;
	SDL_FPoint pos;
	SDL_FPoint speed;
	float zoom;
	float target_zoom;
};
typedef struct Camera Camera;

#define SMOOTHNESS 50.0f
#define BASE_SPEED 2.0f
#define ZOOM_MAX 4.0f
#define ZOOM_MIN 0.1f

extern Camera g_camera;

void camera_update(double dt);

#endif