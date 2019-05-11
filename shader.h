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
};

const char* DefaultShader::vs =
"#version 330 core \n\
in vec3 vPos; \n\
uniform mat4 MVP; \n\
void main() { \n\
   gl_Position = MVP * vec4(vPos, 1.0); \n\
}\n";

const char* DefaultShader::fs =
"#version 330 core \n\
out vec3 color; \n\
void main(){ \n\
  color = vec3(1, 0, 0); \n\
}\n";
