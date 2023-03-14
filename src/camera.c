#include "camera.h"

void cam_speed(camera_t * cam, int vx, int vy){
	cam->speed.x = vx;
	cam->speed.y = vy; 
}

void cam_update(camera_t * camera){
	camera->target_pos.x += camera->speed.x*20;
	camera->target_pos.y += camera->speed.y*20; 
	camera->pos.x += (camera->target_pos.x - camera->pos.x )/10;
	camera->pos.y += (camera->target_pos.y - camera->pos.y )/10;
}