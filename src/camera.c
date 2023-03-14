#include <math.h>
#include "camera.h"
#include "renderer.h"

void cam_speed(Camera * cam, int vx, int vy){
	cam->speed.x = vx;
	cam->speed.y = vy; 
}

void cam_update(Camera * camera, double dt){
	
	float d_zoom = (camera->target_zoom-camera->zoom)*dt/SMOOTHNESS;
	if (d_zoom != 0){
		float zoom_factor = ((d_zoom+camera->zoom)/camera->zoom);
		
		/* Change position scale */
		camera->pos.x *=zoom_factor;
		camera->pos.y *=zoom_factor;
		camera->target_pos.x *=zoom_factor;
		camera->target_pos.y *=zoom_factor;

		/* Add offset to zoom in the mouse cursor */
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		camera->target_pos.x += (float)mouse_x*(d_zoom)/(camera->zoom);
		camera->target_pos.y += (float)mouse_y*(d_zoom)/(camera->zoom);
		camera->pos.x += (float)mouse_x*(d_zoom)/(camera->zoom);
		camera->pos.y += (float)mouse_y*(d_zoom)/(camera->zoom);
		
		camera->zoom += d_zoom;

		/* Improve performance when zoom difference isn't noticeable */
		if (fabs(d_zoom) < 0.0001){
			camera->zoom = camera->target_zoom;
		}		
	}
	

	camera->target_pos.x += camera->speed.x*dt*BASE_SPEED;
	camera->target_pos.y += camera->speed.y*dt*BASE_SPEED; 
	camera->pos.x += (camera->target_pos.x - camera->pos.x )*dt/SMOOTHNESS;
	camera->pos.y += (camera->target_pos.y - camera->pos.y )*dt/SMOOTHNESS;
	
	
}