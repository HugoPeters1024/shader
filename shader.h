#include <stdexcept>

struct Program {
  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint program;
  bool initialized;

  Program() {}
  void Init(const GLchar* const* vs, const GLchar* const* fs)
  {
    vertex_shader = Program::CompileShader(GL_VERTEX_SHADER, vs);
    fragment_shader = Program::CompileShader(GL_FRAGMENT_SHADER, fs);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    initialized = true;
  }

  void Bind() { 
    if (!initialized) 
      throw std::invalid_argument("Initialize program first.");
    glUseProgram(program); 
  }

protected:
  static GLuint CompileShader(GLint type, const GLchar* const* source)
  {
    GLuint shader = glCreateShader(type);
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

      GLchar* errorLog = (GLchar*)malloc(maxLength);
      glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);

      printf("%s", errorLog);

      free(errorLog);
      return -1;
    }

    return shader;
  }
};

struct DefaultShader : Program {
  static const char* vs;
  static const char* fs;
  GLint vPos;
  GLint MVP;

  DefaultShader() : Program() {}
  void Init() {
    Program::Init(&vs, &fs);
    vPos = glGetAttribLocation(program, "vPos");
    MVP  = glGetUniformLocation(program, "MVP");
  }

  void Bind(mat4x4 M) {
    Program::Bind();
    glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*) M);
  }
};

const char* DefaultShader::vs = R"(
#version 330 core
layout(location = 0) in vec3 vPos;
out vec3 fPos;
uniform mat4 MVP; 
void main() {
   gl_Position = MVP * vec4(vPos, 1.0);
   fPos = gl_Position.xyz;
})";

const char* DefaultShader::fs = R"(
#version 330 core
in vec3 fPos;
out vec3 color;
void main(){
  color = vec3(1, 0, 0);
})";
