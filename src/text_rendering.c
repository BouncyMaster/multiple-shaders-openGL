#include <glad/glad.h>
#include <cglm/cglm.h>

#include <ft2build.h>
#include FT_FREETYPE_H 

#include "text_rendering.h"

void
text_rendering_init(const char *vertex_shader_source,
		const char *fragment_shader_source,
		const char *font, struct text_rendering *dest)
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
	FT_Set_Pixel_Sizes(face, 0, 48);
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
text_rendering_perspective(float x, float y, struct text_rendering *text)
{
	glUseProgram(text->shader_program);

	mat4 projection;

	glm_ortho(0, x, 0, y, 0, 1, projection);
	glUniformMatrix4fv(text->projection_loc, 1, GL_FALSE,
			(float *)projection);

	glUseProgram(0);
}

void
text_rendering_render(const char *string, float x, float y, float scale,
		float red, float green, float blue, struct text_rendering *text)
{
	glUseProgram(text->shader_program);
	glBindVertexArray(text->VAO);

	glUniform3f(text->textcolor_loc, red, green, blue);
	while (*string) {
		struct ft_character ch = text->characters[*string++];

		float xpos = x + ch.bearing[0] * scale;
		float ypos = y - (ch.size[1] - ch.bearing[1]) * scale;
		float w = ch.size[0] * scale;
		float h = ch.size[1] * scale;

		float vertices[4][4] = {
			{ xpos + w, ypos + h,  1, 0 },
			{ xpos + w, ypos,      1, 1 },
			{ xpos,     ypos + h,  0, 0 },
			{ xpos,     ypos,      0, 1 },
		};

		glBindTexture(GL_TEXTURE_2D, ch.texture_id);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += ch.advance * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void
text_rendering_cleanup(struct text_rendering *text)
{
	glDeleteVertexArrays(1, &(text->VAO));
	glDeleteBuffers(1, &(text->VBO));
	glDeleteProgram(text->shader_program);
}
