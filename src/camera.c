#include <math.h>
#include "camera.h"
#include "renderer.h"

Camera g_camera = {
	.target_pos = {1000.0f, 1000.0f},
	.pos = {1000.0f, 1000.0f},
	.speed = {0.0f,0.0f},
	.zoom = 1.0f,
	.target_zoom = 1.0f,
};

void cam_update(double dt){
	
	float d_zoom = (g_camera.target_zoom - g_camera.zoom) * dt / SMOOTHNESS;
	if (d_zoom != 0){
		float zoom_factor = (d_zoom + g_camera.zoom) / g_camera.zoom;
		
		/* Change position scale. */
		g_camera.pos.x *= zoom_factor;
		g_camera.pos.y *= zoom_factor;
		g_camera.target_pos.x *= zoom_factor;
		g_camera.target_pos.y *= zoom_factor;

		/* Add offset to zoom in the mouse cursor. */
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		g_camera.target_pos.x += (float)mouse_x * d_zoom / g_camera.zoom;
		g_camera.target_pos.y += (float)mouse_y * d_zoom / g_camera.zoom;
		g_camera.pos.x += (float)mouse_x * d_zoom / g_camera.zoom;
		g_camera.pos.y += (float)mouse_y * d_zoom / g_camera.zoom;
		
		g_camera.zoom += d_zoom;

		/* Improve performance when zoom difference isn't noticeable. */
		if (fabs(d_zoom) < 0.0001f){
			g_camera.zoom = g_camera.target_zoom;
		}		
	}

	g_camera.target_pos.x += g_camera.speed.x*dt*BASE_SPEED;
	g_camera.target_pos.y += g_camera.speed.y*dt*BASE_SPEED; 
	g_camera.pos.x += (g_camera.target_pos.x - g_camera.pos.x )*dt/SMOOTHNESS;
	g_camera.pos.y += (g_camera.target_pos.y - g_camera.pos.y )*dt/SMOOTHNESS;	
}
