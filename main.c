#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "shader.h"
#include "vertexbuf.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description); }

void loop(GLFWwindow* window);

DefaultShader shader;
VertexBuffer quad;
VertexMesh mesh;

float vertices[] = {
  -1.0f,  -1.0f, 1.0f,
  -1.0f,   1.0f, 1.0f,
   1.0f,  -1.0f, 1.0f,

   1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
};

int main() {
  printf("Hello World\n");
  if (!glfwInit()) return -2;
  glfwSetErrorCallback(error_callback);

  // Ensure OpenGL 3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // Create window
  GLFWwindow* window = glfwCreateWindow(640, 480, "Roiboi", NULL, NULL);
  if (!window) {
    printf("Could not create glfw window\n");
    return -3;
  }

  //Bind the window
  glfwMakeContextCurrent(window);

  //Enable vsync
  glfwSwapInterval(1);

  //Setup shaders and vertex buffers
  shader.Init();
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  quad.Init(std::begin(vertices), std::end(vertices), GL_TRIANGLES);
  mesh.Init(8, 8);



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

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  mat4x4_identity(m);
  mat4x4_rotate_Y(m, m, (float)glfwGetTime() / 4.0f);
  mat4x4_ortho(p, -ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
  mat4x4_translate(t, 0, 0, -0.5f);

  mat4x4_mul(mvp, p, m);
  mat4x4_mul(mvp, t, mvp);
  shader.Bind(mvp);
  //quad.Draw();
  mesh.Draw();
}
