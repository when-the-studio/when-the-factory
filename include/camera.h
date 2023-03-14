#ifndef WHEN_THE_FACTORY_CAMERA_
#define WHEN_THE_FACTORY_CAMERA_
#include <SDL2/SDL.h>

#include "map.h"

struct Camera{
	SDL_FPoint target_pos;
	SDL_FPoint pos;
	SDL_FPoint speed;
	float zoom;
	float target_zoom;
};
typedef struct Camera Camera;

#define SMOOTHNESS 50
#define BASE_SPEED 2

extern Camera g_camera;

void cam_speed(float vx, float vy);
void cam_update(double dt);

#endif