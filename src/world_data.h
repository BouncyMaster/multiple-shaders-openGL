#ifndef WORLD_DATA_H
#define WORLD_DATA_H

#include <cglm/cglm.h>

const float triangle[] = {
	//positions	texture coords
	 0,  1,  .2,	.5,  1,
	 1, -1,  .2,	 1,  0,
	-1, -1,  .2,	 0,  0,

	 0,  1, -.2,	.5,  1,
	 1, -1, -.2,	 1,  0,
	-1, -1, -.2,	 0,  0,
	// 3d edges
	 0,  1,  .2,	.2,  1,
	 1, -1,  .2,	.2,  0,
	 0,  1, -.2,	 0,  1,
	 1, -1, -.2,	 0,  0,

	 0,  1, -.2,	.2,  1,
	-1, -1, -.2,	.2,  0,
	 0,  1,  .2,	 0,  1,
	-1, -1,  .2,	 0,  0
};

vec3 positions[] = {
	{ 0.0,  0.0,  0.0},
	{ 2.0,  5.0, -15.0},
	{-1.5, -2.2, -2.5},
	{-3.8, -2.0, -12.3},
	{ 2.4, -0.4, -3.5},
	{-1.7,  3.0, -7.5},
	{ 1.3, -2.0, -2.5},
	{ 1.5,  2.0, -2.5},
	{ 1.5,  0.2, -1.5},
	{-1.3,  1.0, -1.5}
};

#endif
