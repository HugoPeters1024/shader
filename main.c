#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLFW/glfw3.h>

#include "linmath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.h"
#include "vertexbuf.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description); }

void APIENTRY gl_debug_output(const GLenum source, const GLenum type, const GLuint id, const GLenum severity, const GLsizei, const GLchar *message, void *) {
	// Skip buffer info messages, framebuffer info messages, texture usage state warning, redundant state change buffer
	if (id == 131185 // ?
		|| id == 131169 // ?
		|| id == 131204 // ?
		|| id == 8 // ?
		|| id == 131218) // Unexplainable performance warnings
	{
		return;
	}

	//if (id == 131218) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source) {
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		default: break;
	}
	std::cout << std::endl;

	switch (type) {
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		default: break;
	}
	std::cout << std::endl;

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		default: break;
	}
	std::cout << std::endl;
}

void loop(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
float time_correction = 0.0f;

VertexMesh mesh;
Quad quad;

float vertices[] = {
  -1.0f,  -1.0f, 1.0f,
  -1.0f,   1.0f, 1.0f,
   1.0f,  -1.0f, 1.0f,

   1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
};

int main() {
  if (!glfwInit()) return -2;
  glfwSetErrorCallback(error_callback);

  // Ensure OpenGL 4
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  // Create window
  GLFWwindow* window = glfwCreateWindow(640, 480, "Roiboi", NULL, NULL);
  if (!window) {
    printf("Could not create glfw window\n");
    return -3;
  }

  //Bind the window
  glfwMakeContextCurrent(window);

  //Enable debugging
  glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GLDEBUGPROC(gl_debug_output), nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);


  //Set key callback
  glfwSetKeyCallback(window, key_callback);

  // Print info
  const GLubyte* vendor = glGetString(GL_VENDOR);
  printf("Video card:\t%s\n", vendor);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  printf("Renderer:\t%s\n", renderer);
  const GLubyte* version = glGetString(GL_VERSION);
  printf("OpenGL version:\t%s\n", version);
  printf("------------\n");

  //Enable vsync
  glfwSwapInterval(1);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  mesh.Init(20, 20);
  quad.Init();


  while(!glfwWindowShouldClose(window)) 
  {
    loop(window);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}


void loop(GLFWwindow* window) {
  float ratio;
  int width, height;
  mat4x4 m, p, t, s, mvp;

  glfwGetFramebufferSize(window, &width, &height);
  ratio = width / (float)height;

  mat4x4_identity(m);
  mat4x4_rotate_X(m, m, 2.2f);
  mat4x4_rotate_Z(m, m, glfwGetTime() - time_correction);
  mat4x4_ortho(p, -ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
  mat4x4_translate(t, 0, 0, 0);

  mat4x4_mul(mvp, p, m);
  mat4x4_mul(mvp, t, mvp);

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  quad.Draw(glfwGetTime());
  mesh.Draw(mvp, time_correction);

  if (time_correction > 0) 
    time_correction -= 0.01f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == 32 && action == GLFW_RELEASE) { // space
    time_correction += 0.5f;
    printf("whoop whoop\n");
  }
}
