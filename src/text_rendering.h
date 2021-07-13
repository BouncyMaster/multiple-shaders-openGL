#ifndef TEXT_RENDERING_H
#define TEXT_RENDERING_H

struct ft_character {
	unsigned int texture_id;
	short size[2];
	short bearing[2];
	short advance;
};

struct text_rendering {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int shader_program;
	int projection_loc;
	int textcolor_loc;
	struct ft_character characters[128];
};

void text_rendering_init(const char *vertex_shader_source,
		const char *fragment_shader_source,
		const char *font, struct text_rendering *dest);

void text_rendering_perspective(float x, float y, struct text_rendering *text);

void text_rendering_render(const char *string, float x, float y, float scale,
		float red, float green, float blue,
		struct text_rendering *text);

void text_rendering_cleanup(struct text_rendering *text);

#endif
