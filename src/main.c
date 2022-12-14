#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <file_ops.h>
#include "world_data.h"
#include "text.h"
#include "camera.h"

float top_time = 0;

short screen_size[2] = {800, 800};
bool screen_changed = true;

bool mouse_first = true, zoom_changed = true, is_windowed = true;
float mouse_last[2] = {400, 400}; // screen_size / 2

struct camera main_camera;

void
framebuffer_size_callback(GLFWwindow *window, int w, int h)
{
	glViewport(0, 0, w, h);

	screen_changed = true;

	screen_size[0] = w;
	screen_size[1] = h;
}

void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_F:
		{
			GLFWmonitor *monitor = NULL;
			if (is_windowed)
				monitor = glfwGetPrimaryMonitor();

			glfwSetWindowMonitor(window, monitor, 0, 0, 1600, 900,
						GLFW_DONT_CARE);
			is_windowed = !is_windowed;
			break;
		}
		case GLFW_KEY_SPACE:
			top_time = 0;
		}
	}
}

void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (mouse_first) {
		mouse_last[0] = xpos;
		mouse_last[1] = ypos;
		mouse_first = false;
	}

	vec2 offset = {xpos - mouse_last[0], mouse_last[1] - ypos};

	mouse_last[0] = xpos;
	mouse_last[1] = ypos;

	camera_process_mouse(offset, &main_camera);
}

void
scroll_callback(GLFWwindow* window, double offsetX, double offsetY)
{
	zoom_changed = true;
	camera_process_mousescroll((short)offsetY, &main_camera);
}

int
main(void)
{
	unsigned int world_VAO, world_VBO, world_shader, texture;
	float delta_time, last_frame = 0;
	struct text main_text;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(screen_size[0], screen_size[1],
			"multiple-shaders", NULL, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glGenVertexArrays(1, &world_VAO);
	glGenBuffers(1, &world_VBO);

	glBindVertexArray(world_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, world_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle,
			GL_STATIC_DRAW);

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	// texture coord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
			(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	{
	int width, height, nr_channels;

	unsigned char *data = stbi_load("data/wall.jpg", &width, &height,
			&nr_channels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	char *vertex_source, *fragment_source;

	vertex_source = file_to_str("shaders/world.vs");
	fragment_source = file_to_str("shaders/world.fs");

	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, (const char **)&vertex_source, NULL);
	glCompileShader(vertex);

	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, (const char **)&fragment_source, NULL);
	glCompileShader(fragment);

	world_shader = glCreateProgram();
	glAttachShader(world_shader, vertex);
	glAttachShader(world_shader, fragment);
	glLinkProgram(world_shader); 

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	file_to_str_free(vertex_source);
	file_to_str_free(fragment_source);

	camera_init((vec3){0, 0, 3}, (vec3){0, 1, 0}, &main_camera);

	vertex_source = file_to_str("shaders/text-rendering.vs");
	fragment_source = file_to_str("shaders/text-rendering.fs");
	text_init(
		vertex_source, fragment_source,
		"/usr/local/share/fonts/Liberation/LiberationSans-Regular.ttf",
		&main_text
	);

	file_to_str_free(vertex_source);
	file_to_str_free(fragment_source);
	}

	mat4 projection, model, view, pv, pvm;

	int pvm_loc = glGetUniformLocation(world_shader, "pvm");

	char frametime_str[50];

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(.1, .1, .1, .3);

	float current_frame;
	while(!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera_process_keyboard(FORWARD, delta_time,
					&main_camera);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera_process_keyboard(BACKWARD, delta_time,
					&main_camera);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera_process_keyboard(LEFT, delta_time,
					&main_camera);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera_process_keyboard(RIGHT, delta_time,
					&main_camera);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(screen_changed || zoom_changed) {
			if (screen_changed)
				text_perspective(
					(vec2){screen_size[0], screen_size[1]},
					&main_text
				);

			glm_perspective(
				glm_rad(main_camera.zoom),
				(float)screen_size[0]/(float)screen_size[1],
				.1, 100, projection
			);

			screen_changed = false;
			zoom_changed = false;
		}

		glUseProgram(world_shader);
		glBindVertexArray(world_VAO);
		glBindTexture(GL_TEXTURE_2D, texture);

		camera_getviewmatrix(view, &main_camera);
		glm_mat4_mul(projection, view, pv);

		for (short i = 0; i < sizeof(positions)/sizeof(vec3); ++i) {
			glm_translate_make(model, (float *)positions[i]);
			glm_rotate_y(model, cos(current_frame), model);
			glm_rotate_x(model, glm_rad(20 * i), model);

			glm_mat4_mul(pv, model, pvm);
			glUniformMatrix4fv(pvm_loc, 1, GL_FALSE,
				(float *)pvm);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDrawArrays(GL_TRIANGLE_STRIP, 6, 8);
			glDrawArrays(GL_TRIANGLE_STRIP, 14, 4);
		}

		glUseProgram(0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		if (delta_time > top_time)
			top_time = delta_time;

		sprintf(frametime_str, "Frametime: %.2fms", delta_time*1000);
		text_render(frametime_str, (vec2){15, 35}, 1,
				(vec3){.9, .2, .2}, &main_text);

		sprintf(frametime_str, "Top time: %.2fms", top_time*1000);
		text_render(frametime_str, (vec2){15, 15}, 1,
				(vec3){.9, .4, .4}, &main_text);
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	text_cleanup(&main_text);

	glDeleteVertexArrays(1, &world_VAO);
	glDeleteBuffers(1, &world_VBO);
	glDeleteProgram(world_shader);

	glfwTerminate();
}
