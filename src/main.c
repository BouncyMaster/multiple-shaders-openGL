#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "world_data.h"
#include "file_ops.h"
#include "text_rendering.h"
#include "camera.h"

unsigned int VAO, VBO, shader_program;
float delta_time, last_frame = 0;
float lastX = 400, lastY = 400; // scr_width / 2
bool first_mouse = true, zoom_changed = true;
struct camera main_camera;

struct screen {
	short size[2];
	bool screen_changed;
} main_screen = {{800, 800}, true};

void
framebuffer_size_callback(GLFWwindow *window, int w, int h)
{
	glViewport(0, 0, w, h);

	main_screen.screen_changed = true;

	main_screen.size[0] = w;
	main_screen.size[1] = h;
}

void
keyboard_callback(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

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
}

void
mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (first_mouse) {
		lastX = xpos;
		lastY = ypos;
		first_mouse = false;
	}

	vec2 offset = {xpos - lastX, lastY - ypos};

	lastX = xpos;
	lastY = ypos;

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
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(main_screen.size[0],
			main_screen.size[1], "multiple-shaders", 0, 0);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nr_channels;

	unsigned char *data = stbi_load("data/wall.jpg", &width, &height,
			&nr_channels, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	char *vertex_source, *fragment_source;

	vertex_source = file_to_str("shaders/world.vs");
	fragment_source = file_to_str("shaders/world.fs");

	unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertex_source, NULL);
	glCompileShader(vertex);

	unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragment_source, NULL);
	glCompileShader(fragment);

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex);
	glAttachShader(shader_program, fragment);
	glLinkProgram(shader_program); 

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	file_to_str_cleanup(vertex_source);
	file_to_str_cleanup(fragment_source);

	camera_init((vec3){0, 0, 3}, (vec3){0, 1, 0}, &main_camera);

	struct text_rendering main_text;

	vertex_source = file_to_str("shaders/text-rendering.vs");
	fragment_source = file_to_str("shaders/text-rendering.fs");
	text_rendering_init(
		vertex_source, fragment_source,
		"/usr/share/fonts/liberation-fonts/LiberationSans-Regular.ttf",
		&main_text
	);

	file_to_str_cleanup(vertex_source);
	file_to_str_cleanup(fragment_source);

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

	mat4 model, view, projection;

	int model_loc = glGetUniformLocation(shader_program, "model");
	int view_loc = glGetUniformLocation(shader_program, "view");
	int projection_loc = glGetUniformLocation(shader_program, "projection");

	glEnable(GL_DEPTH_TEST);
	glClearColor(.2, .3, .3, 1);

	float current_frame;
	while(!glfwWindowShouldClose(window)) {
		keyboard_callback(window);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(main_screen.screen_changed || zoom_changed) {
			main_screen.screen_changed = false;
			zoom_changed = false;

			glm_perspective(glm_rad(main_camera.zoom),
				(float)main_screen.size[0]/(float)main_screen.size[1], .1, 100, projection);
			glUseProgram(shader_program);
			glUniformMatrix4fv(projection_loc, 1, GL_FALSE,
				(float *)projection);
			glUseProgram(0);


			text_rendering_perspective(
				(vec2){main_screen.size[0],
					main_screen.size[1]},
				&main_text
			);
		}


		glUseProgram(shader_program);
		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, texture);

		camera_getviewmatrix(view, &main_camera);
		glUniformMatrix4fv(view_loc, 1, GL_FALSE,
			(float *)view);

		for (short i = 0; i < sizeof(positions)/sizeof(vec3); ++i) {
			glm_translate_make(model, positions[i]);
			glm_rotate_y(model, cos(current_frame), model);
			glm_rotate_x(model, glm_rad(20 * i), model);

			glUniformMatrix4fv(model_loc, 1, GL_FALSE,
				(float *)model);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDrawArrays(GL_TRIANGLE_STRIP, 6, 8);
		}
		glUseProgram(0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

		text_rendering_render("This is sample text", (vec2){25, 25}, 1,
				(vec3){.5, .8, .2}, &main_text);

		text_rendering_render("Mean3Voice-amid!!", (vec2){500, 700},
				.75, (vec3){.8, .2, .5}, &main_text);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	text_rendering_cleanup(&main_text);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shader_program);

	glfwTerminate();
}
