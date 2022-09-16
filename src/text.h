#ifndef TEXT_H
#define TEXT_H

#include <cglm/cglm.h>

struct ft_character {
	unsigned int texture_id;
	short size[2];
	short bearing[2];
	short advance;
};

struct text {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int shader_program;
	int projection_loc;
	int textcolor_loc;
	struct ft_character characters[128];
};

void text_init(const char *vertex_shader_source,
		const char *fragment_shader_source,
		const char *font, struct text *dest);

void text_perspective(vec2 size, struct text *Text);

void text_render(const char *string, vec2 pos, float scale,
		vec3 color, struct text *Text);

void text_cleanup(struct text *Text);

#endif
