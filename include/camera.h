#ifndef WHEN_THE_FACTORY_CAMERA_
#define WHEN_THE_FACTORY_CAMERA_
#include <SDL2/SDL.h>

#include "map.h"

struct Camera{
	Coord target_pos;
	Coord pos;
	Coord speed;
	int zoom;
};

#define SMOOTHNESS 50;
#define BASE_SPEED 2;


typedef struct Camera Camera;

void cam_speed(Camera* cam, int vx, int vy);
void cam_update(Camera* cam, double dt);

#endif