#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#include <GLFW/glfw3.h>

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description); }

void loop();
void GL_Setup();
GLint CompileShader();

GLuint vertex_buffer, vertex_shader, fragment_shader, program, vao;
GLint mvp_location, vpos_location, vcol_location, test_location;

float vertices[] = {
  -1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
  1.0f,  -1.0f, 1.0f,

  1.0f, -1.0f, 1.0f,
  -1.0f,  1.0f, 1.0f,
  1.0f,  1.0f, 1.0f,
};

static const char* vertex_shader_text =
"#version 330 core \n\
in vec3 vPos; \n\
void main() { \n\
   gl_Position = vec4(vPos, 1.0); \n\
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

  glfwGetFramebufferSize(window, &width, &height);
  ratio = width / (float)height;

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);
  //glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glUseProgram(program);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  printf("OpenGL err: %i\n", glGetError());
}

void GL_Setup() {
  glGenBuffers(1, &vertex_buffer);
  glGenVertexArrays(1, &vao);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(vao);

  vertex_shader = CompileShader(GL_VERTEX_SHADER, &vertex_shader_text);
  fragment_shader = CompileShader(GL_FRAGMENT_SHADER, &fragment_shader_text);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  vpos_location = glGetAttribLocation(program, "vPos");
  printf("vPos location: %i\n", vpos_location);

  glEnableVertexAttribArray(vpos_location);
  printf("OpenGL err: %i\n", glGetError());
  glVertexAttribPointer(
      vpos_location,
      3, // elements per vertex
      GL_FLOAT,
      GL_FALSE, // normalized
      0,
      (void*)(sizeof(float) * 0));
  printf("OpenGL err: %i\n", glGetError());
}

GLint CompileShader(GLint type, const GLchar* const* source)
{
  GLint shader = glCreateShader(type);
  glShaderSource(shader, 1, source, NULL);
  glCompileShader(shader);

  // Check the compilation of the shader 
  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  printf("Shader Compilation:\t\t");
  if (success) printf("success\n"); else printf("failed\n");
  if (success == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    GLchar* errorLog = malloc(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);

    printf("%s", errorLog);

    free(errorLog);
    //glDeleteShader(shader);

    abort();
    return -1;
  }

  return shader;
}
