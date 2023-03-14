#ifndef WHEN_THE_FACTORY_CAMERA_
#define WHEN_THE_FACTORY_CAMERA_
#include <SDL2/SDL.h>

#include "map.h"

struct camera_t{
	coord_t target_pos;
	coord_t pos;
	coord_t speed;
	int zoom;
};

typedef struct camera_t camera_t;

void cam_speed(camera_t * cam, int vx, int vy);
void cam_update(camera_t * cam);

#endif