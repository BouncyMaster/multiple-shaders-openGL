#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "file_ops.h"
#include "text_rendering.h"

struct screen {
	short size[2];
	bool perspective_changed;
} main_screen = {{800, 800}, true};

void
framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);

	main_screen.perspective_changed = true;

	main_screen.size[0] = w;
	main_screen.size[1] = h;
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

	struct text_rendering main_text;

	char *vertex_source = file_to_str("shaders/vertex.glsl");
	char *fragment_source = file_to_str("shaders/fragment.glsl");
	text_rendering_init(
		vertex_source, fragment_source,
		"/usr/share/fonts/liberation-fonts/LiberationSans-Regular.ttf",
		&main_text
	);

	file_to_str_cleanup(vertex_source);
	file_to_str_cleanup(fragment_source);

	while(!glfwWindowShouldClose(window)) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, 1);

		glClearColor(.2, .3, .3, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		if (main_screen.perspective_changed) {
			main_screen.perspective_changed = false;
			text_rendering_perspective((float)main_screen.size[0], 
					(float)main_screen.size[1], &main_text);
		}

		text_rendering_render("This is sample text", 25, 25, 1,
				.5, .8, .2, &main_text);

		text_rendering_render("Mean3Voice-amid!!", 500, 700, .75,
				.8, .2, .5, &main_text);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	text_rendering_cleanup(&main_text);
	glfwTerminate();
}
