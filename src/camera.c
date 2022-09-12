#include "camera.h"
#include <math.h>

static void
camera_updatevectors(struct camera *cam)
{
	vec3 front;
	// calculate the new front vector
	front[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
	front[1] = sin(glm_rad(cam->pitch));
	front[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
	glm_vec3_copy(front, cam->front);

	// also re-calculate the right and up vector
	glm_vec3_crossn(cam->front, cam->world_up, cam->right);
	glm_vec3_crossn(cam->right, cam->front, cam->up);
}

void
camera_init(vec3 position, vec3 up, struct camera *dest)
{
	glm_vec3_copy(position, dest->position);
	glm_vec3_copy(up, dest->world_up);
	dest->yaw = YAW;
	dest->pitch = PITCH;
	dest->constrain_pitch = true;
	dest->movement_speed = SPEED;
	dest->mouse_sensitivity = SENSITIVITY;
	dest->zoom = ZOOM;

	camera_updatevectors(dest);
}

void
camera_getviewmatrix(mat4 out, struct camera *cam)
{
	vec3 center;
	glm_vec3_add(cam->position, cam->front, center);
	glm_lookat(cam->position, center, cam->up, out);
}

void
camera_process_keyboard(enum camera_movement direction, float delta_time,
		struct camera *cam)
{
	float velocity = cam->movement_speed * delta_time;
	vec3 position_delta;

	switch (direction) {
	case FORWARD:
		glm_vec3_scale(cam->front, velocity, position_delta);
		glm_vec3_add(cam->position, position_delta, cam->position);
		break;
	case BACKWARD:
		glm_vec3_scale(cam->front, velocity, position_delta);
		glm_vec3_sub(cam->position, position_delta, cam->position);
		break;
	case LEFT:
		glm_vec3_scale(cam->right, velocity, position_delta);
		glm_vec3_sub(cam->position, position_delta, cam->position);
		break;
	case RIGHT:
		glm_vec3_scale(cam->right, velocity, position_delta);
		glm_vec3_add(cam->position, position_delta, cam->position);
		break;
	}
}

void
camera_process_mouse(vec2 offset, struct camera *cam)
{
	glm_vec2_scale(offset, cam->mouse_sensitivity, offset);

	cam->yaw += offset[0];
	cam->pitch += offset[1];

	if (cam->constrain_pitch) {
		if (cam->pitch > 89)
			cam->pitch = 89;
		else if (cam->pitch < -89)
			cam->pitch = -89;
	}

	camera_updatevectors(cam);
}

void
camera_process_mousescroll(short offset, struct camera *cam)
{
	cam->zoom -= offset;

	if (cam->zoom < 1)
		cam->zoom = 1;
	else if (cam->zoom > 45)
		cam->zoom = 45;
}
