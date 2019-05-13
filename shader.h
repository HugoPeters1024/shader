#include <stdexcept>

inline static GLuint CompileShader(GLint type, const GLchar* const* source)
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
};

struct DefaultShader {
  static const char* vs;
  static const char* fs;
  GLint vPos, vNormal, MVP, iTime;
  GLuint vertex_shader, fragment_shader;
  GLuint program;

  DefaultShader() {}
  void Init() {
    vertex_shader = CompileShader(GL_VERTEX_SHADER, &vs);
    fragment_shader = CompileShader(GL_FRAGMENT_SHADER, &fs);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    vPos = glGetAttribLocation(program, "vPos");
    vNormal = glGetAttribLocation(program, "vNormal");
    MVP  = glGetUniformLocation(program, "MVP");
    iTime  = glGetUniformLocation(program, "iTime");
  }

  void Bind(mat4x4 mvp) {
    glUseProgram(program);
    glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*) mvp);
    glUniform1f(iTime, glfwGetTime());
  }
};

const char* DefaultShader::vs = R"(
#version 330 core
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
out vec3 fColor;
uniform mat4 MVP; 
uniform float iTime;
void main() {
   vec3 lightDir = vec3(1, 0, 0);
   vec3 normal = (MVP * vec4(vNormal, 1)).xyz;
   float theta = max(dot(normal, lightDir),0);
   gl_Position =  MVP * vec4(vPos, 1.0);
   fColor = vec3(1) * (theta + 0.2f);
})";

const char* DefaultShader::fs = R"(
#version 330 core
in vec3 fColor;
out vec3 color;
void main(){
  color = fColor;
})";

struct ComputeShader {
  bool initialized;
  GLuint program;
  GLuint compute_shader;
  GLuint input_buffer;
  GLuint output_buffer;
  static const char* src;

  ComputeShader() {}
  void Init() {
    program = glCreateProgram();
    compute_shader = CompileShader(GL_COMPUTE_SHADER, &src);
    glAttachShader(program, compute_shader);
    glLinkProgram(program);


    // Generate input buffer
    glGenBuffers(1, &input_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * sizeof(float), NULL, GL_STATIC_DRAW);

    // Map input buffer and fill it
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; 
    float* data = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 64 * sizeof(float), bufMask);

    for(int i=0; i<64; i++)
    {
      data[i] = (float)i;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


    // Generate outpt buffer and allocate size
    glGenBuffers(1, &output_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * sizeof(float), NULL, GL_STATIC_DRAW);
  }

  void Run() { 
    // Match buffer objects to shader mounts
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, input_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, output_buffer);

    glUseProgram(program); 
    // The shader uses an 8x8 group, so only one is required.
    glDispatchCompute(1, 1, 1);
    
    // Ensure all writes of previous shader have completed
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Mount the output buffer and print it
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
    float* data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    for(int i=0; i<64; i++)
    {
      printf("data[%i]: %f\n", i, data[i]);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    GLint success = glGetError();
    printf("ERROR CODE: %i\n", success);
  }
};

const char* ComputeShader::src = R"(
# version 430 core
layout(local_size_x = 8, local_size_y = 8) in;
layout(std430, binding=4) buffer inBuf
{
  vec4 input_data[];
};

layout(std430, binding=5) buffer outBuf
{
  vec4 output_data[];
};

void main() {
  uint pos = gl_GlobalInvocationID.x + 8 * gl_GlobalInvocationID.y;
  output_data[pos].x = gl_GlobalInvocationID.x / 8.0f;
  output_data[pos].y = gl_GlobalInvocationID.y / 8.0f;
  output_data[pos].z = 0;
  output_data[pos].w = -1;
}
)";
