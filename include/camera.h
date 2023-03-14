#ifndef WHEN_THE_FACTORY_CAMERA_
#define WHEN_THE_FACTORY_CAMERA_
#include <SDL2/SDL.h>

#include "map.h"

struct Camera{
	SDL_FPoint target_pos;
	SDL_FPoint pos;
	Coord speed;
	float zoom;
	float targetZoom;
};

#define SMOOTHNESS 50;
#define BASE_SPEED 2;


typedef struct Camera Camera;

void cam_speed(Camera* cam, int vx, int vy);
void cam_update(Camera* cam, double dt);

#endif