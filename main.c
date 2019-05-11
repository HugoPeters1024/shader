#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "shader.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description); }

void loop(GLFWwindow* window);
void GL_Setup();

GLuint vertex_buffer, vertex_shader, fragment_shader, program, vao;
GLint mvp_location, vpos_location, vcol_location, test_location;
DefaultShader shader;

float vertices[] = {
  -1.0f,  -1.0f, 1.0f,
  -1.0f,   1.0f, 1.0f,
   1.0f,  -1.0f, 1.0f,

   1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
   1.0f,  1.0f, 1.0f,
};

static const char* vertex_shader_text =
"#version 330 core \n\
in vec3 vPos; \n\
uniform mat4 MVP; \n\
void main() { \n\
   gl_Position = MVP * vec4(vPos, 1.0); \n\
}\n";

static const char* fragment_shader_text = 
"#version 330 core \n\
out vec3 color; \n\
void main(){ \n\
  color = vec3(1, 0, 0); \n\
}\n";


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

  GL_Setup();


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
  mat4x4 m, p, t, mvp;

  glfwGetFramebufferSize(window, &width, &height);
  ratio = width / (float)height;

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  mat4x4_identity(m);
  mat4x4_rotate_Z(m, m, (float)glfwGetTime());
  mat4x4_ortho(p, -ratio, ratio, -1.0f, 1.0f, 1.0f, -1.0f);
  mat4x4_translate(t, 0, 0, -0.5f);
  mat4x4_mul(mvp, p, m);
  mat4x4_mul(mvp, t, mvp);
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

  shader.Bind();
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void GL_Setup() {
  glGenBuffers(1, &vertex_buffer);
  glGenVertexArrays(1, &vao);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(vao);

  shader.Init();

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(
      vpos_location,
      3, // elements per vertex
      GL_FLOAT,
      GL_FALSE, // normalized
      0,
      (void*)(sizeof(float) * 0));

  GLint err = glGetError();
  if (err != 0) printf("ERROR CODE: %i\n", err);
}
