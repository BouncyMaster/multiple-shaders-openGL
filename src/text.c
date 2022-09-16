#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H 

#include "text.h"

void
text_init(const char *vertex_shader_source,
		const char *fragment_shader_source,
		const char *font, struct text *dest)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &(dest->VAO));
	glGenBuffers(1, &(dest->VBO));

	glBindVertexArray(dest->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, dest->VBO);
	// Allocate for a quad with 2 position and 2 texture coords
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, NULL,
			GL_DYNAMIC_DRAW);

	// position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	// texture coord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			(void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	FT_Library ft;
	FT_Init_FreeType(&ft);

	FT_Face face;
	FT_New_Face(ft, font, 0, &face);
	FT_Set_Pixel_Sizes(face, 0, 24);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int texture;
	for (unsigned char c = 0; c < 128; ++c) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			continue;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_BORDER);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		dest->characters[c].texture_id = texture;
		dest->characters[c].size[0] = face->glyph->bitmap.width;
		dest->characters[c].size[1] = face->glyph->bitmap.rows;
		dest->characters[c].bearing[0] = face->glyph->bitmap_left;
		dest->characters[c].bearing[1] = face->glyph->bitmap_top;
		dest->characters[c].advance = face->glyph->advance.x >> 6;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	unsigned int vertex_shader, fragment_shader;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	dest->shader_program = glCreateProgram();
	glAttachShader(dest->shader_program, vertex_shader);
	glAttachShader(dest->shader_program, fragment_shader);
	glLinkProgram(dest->shader_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	dest->projection_loc = glGetUniformLocation(dest->shader_program,
			"projection");
	dest->textcolor_loc = glGetUniformLocation(dest->shader_program,
			"textColor");
}

void
text_perspective(vec2 size, struct text *Text)
{
	glUseProgram(Text->shader_program);

	mat4 projection;

	glm_ortho(0, size[0], 0, size[1], 0, 1, projection);
	glUniformMatrix4fv(Text->projection_loc, 1, GL_FALSE,
			(float *)projection);

	glUseProgram(0);
}

void
text_render(const char *string, vec2 pos, float scale, vec3 color,
		struct text *Text)
{
	struct ft_character ch;
	float xpos, ypos, w, h;
	float vertices[4][4] = {
		{0, 0, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 1, 1},
		{0, 0, 0, 1}
	};

	glUseProgram(Text->shader_program);
	glBindVertexArray(Text->VAO);

	glUniform3f(Text->textcolor_loc, color[0], color[1], color[2]);
	while (*string) {
		ch = Text->characters[*string++];

		xpos = pos[0] + ch.bearing[0] * scale;
		ypos = pos[1] - (ch.size[1] - ch.bearing[1]) * scale;
		w = ch.size[0] * scale;
		h = ch.size[1] * scale;

		vertices[0][0] = xpos + w;
		vertices[0][1] = ypos + h;

		vertices[1][0] = xpos;
		vertices[1][1] = vertices[0][1];

		vertices[2][0] = vertices[0][0];
		vertices[2][1] = ypos;

		vertices[3][0] = xpos;
		vertices[3][1] = ypos;

		glBindTexture(GL_TEXTURE_2D, ch.texture_id);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		pos[0] += ch.advance * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void
text_cleanup(struct text *Text)
{
	glDeleteVertexArrays(1, &(Text->VAO));
	glDeleteBuffers(1, &(Text->VBO));
	glDeleteProgram(Text->shader_program);
}
