#ifndef WORLD_DATA_H
#define WORLD_DATA_H

const float square[] = {
	//positions	texture coords
	 1,  1, 0,	1, 1,
	 1, -1, 0,	1, 0,
	-1,  1, 0,	0, 1,
	-1, -1, 0,	0, 0
};

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

#endif
