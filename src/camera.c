#include <math.h>
#include "camera.h"
#include "renderer.h"

void cam_speed(Camera * cam, int vx, int vy){
	cam->speed.x = vx;
	cam->speed.y = vy; 
}

void cam_update(Camera * camera, double dt){
	
	double dZoom = (camera->targetZoom-camera->zoom)*dt/SMOOTHNESS;
	if (dZoom != 0){
		double zoomFactor = ((dZoom+camera->zoom)/camera->zoom);
		
		// Change position scale
		camera->pos.x *=zoomFactor;
		camera->pos.y *=zoomFactor;
		camera->target_pos.x *=zoomFactor;
		camera->target_pos.y *=zoomFactor;
		
		// Add offset to zoom in the middle of the screen
		camera->target_pos.x += WINDOW_W*(dZoom)/(2*camera->zoom);
		camera->target_pos.y += WINDOW_H*(dZoom)/(2*camera->zoom);
		camera->pos.x += WINDOW_W*(dZoom)/(2*camera->zoom);
		camera->pos.y += WINDOW_H*(dZoom)/(2*camera->zoom);
		
		camera->zoom += dZoom;

		// Improve performance when zoom difference isn't noticeable
		if (fabs(dZoom) < 0.0001){
			camera->zoom = camera->targetZoom;
		}		
	}
	

	camera->target_pos.x += camera->speed.x*dt*BASE_SPEED;
	camera->target_pos.y += camera->speed.y*dt*BASE_SPEED; 
	camera->pos.x += (camera->target_pos.x - camera->pos.x )*dt/SMOOTHNESS;
	camera->pos.y += (camera->target_pos.y - camera->pos.y )*dt/SMOOTHNESS;
	
	
}