#include "camera.h"

void cam_speed(Camera * cam, int vx, int vy){
	cam->speed.x = vx;
	cam->speed.y = vy; 
}

void cam_update(Camera * camera, double dt){
	camera->target_pos.x += camera->speed.x*dt*BASE_SPEED;
	camera->target_pos.y += camera->speed.y*dt*BASE_SPEED; 
	camera->pos.x += (camera->target_pos.x - camera->pos.x )*dt/SMOOTHNESS;
	camera->pos.y += (camera->target_pos.y - camera->pos.y )*dt/SMOOTHNESS;
}