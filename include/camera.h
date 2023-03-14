#ifndef WHEN_THE_FACTORY_CAMERA_
#define WHEN_THE_FACTORY_CAMERA_
#include <SDL2/SDL.h>

#include "map.h"

struct Camera{
	coord_t target_pos;
	coord_t pos;
	coord_t speed;
	int zoom;
};

#define SMOOTHNESS 10;
#define BASE_SPEED 20;

typedef struct Camera Camera;

void cam_speed(Camera* cam, int vx, int vy);
void cam_update(Camera* cam);

#endif