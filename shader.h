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
  GLuint source;
  GLuint target;
  size_t buf_size;
  static const char* src;

  ComputeShader() {}
  void Init(GLuint source, GLuint target, size_t buf_size) {
    this->source = source;
    this->target = target;
    this->buf_size = buf_size;
    program = glCreateProgram();
    compute_shader = CompileShader(GL_COMPUTE_SHADER, &src);
    glAttachShader(program, compute_shader);
    glLinkProgram(program);


    // Generate input buffer
    glGenBuffers(1, &input_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, buf_size, NULL, GL_STATIC_DRAW);

    // Map input buffer and fill it
    /*
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; 
    float* data = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buf_size, bufMask);

    for(int i=0; i<64; i++)
    {
      //data[i] = (float)i;
      data[i] = -429;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    */


    // Generate outpt buffer and allocate size
    glGenBuffers(1, &output_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, buf_size, NULL, GL_STATIC_DRAW);
  }

  void Run() { 
    printf("source buffer ID: %i\n", source);
    printf("target buffer ID: %i\n", target);

    // Copy source data to the shader data
    glCopyNamedBufferSubData(source, input_buffer, 0, 0, buf_size);

    // Match buffer objects to shader mounts
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, input_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, output_buffer);

    glUseProgram(program); 
    // The shader uses an 8x8 group, so only one is required.
    glDispatchCompute(buf_size / sizeof(vec3), 1, 1);
    
    // Ensure all writes of previous shaders have completed
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Copy result back to output buffer
    glCopyNamedBufferSubData(output_buffer, target, 0, 0, buf_size);

    // Mount the result buffer and print it
    glBindBuffer(GL_ARRAY_BUFFER, output_buffer);
    vec3* data = (vec3*)glMapBuffer(GL_ARRAY_BUFFER , GL_READ_ONLY);
    if ((unsigned long)data == 0)
    {
      printf("Got null pointer\n");
     // abort();
    }

    for(int i=0; i<buf_size / (3 * sizeof(float)); i++)
    {
      printf("data[%i]: (%f, %f, %f)\n", i, data[i][0], data[i][1], data[i][2]);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    GLint success = glGetError();
    printf("ERROR CODE: %i\n", success);
  }
};

const char* ComputeShader::src = R"(
# version 430 core
layout(local_size_x = 1, local_size_y = 1) in;
layout(std430, binding=4) buffer inBuf
{
  vec4 input_data[];
};

layout(std430, binding=5) buffer outBuf
{
  vec4 output_data[];
};

void main() {
  uint pos = gl_GlobalInvocationID.x;
  output_data[pos].x = input_data[pos].x;//-input_data[pos];
}
)";
