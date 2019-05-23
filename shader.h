#include <stdexcept>

#define WORK_GROUP_SIZE 8

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

struct BareShader {
  static const char* vs;
  static const char* fs;
  GLint vPos, uvPos;
  GLuint vertex_shader, fragment_shader;
  GLuint program;

  BareShader() {}
  void Init() {
    vertex_shader = CompileShader(GL_VERTEX_SHADER, &vs);
    fragment_shader = CompileShader(GL_FRAGMENT_SHADER, &fs);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    vPos = glGetAttribLocation(program, "vPos");
    uvPos = glGetAttribLocation(program, "uvPos");
  }

  void Bind() {
    glUseProgram(program);
  }
};

const char* BareShader::vs = R"(
#version 330 core
layout(location = 0) in vec3 vPos;
layout(location = 0) in vec2 uvPos;

out vec2 texCoord;

void main() {
   gl_Position = vec4(vPos, 1);
   texCoord = uvPos;
})";

const char* BareShader::fs = R"(
#version 330 core
in vec2 texCoord;
out vec4 color;

uniform sampler2D tex;

void main() {
  vec2 sample = vec2(texCoord.x + 0.05f * sin(texCoord.x * 10), texCoord.y);
  color = texture(tex, sample);
})";

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

  void Bind(mat4x4 mvp, float time_correction) {
    glUseProgram(program);
    glUniformMatrix4fv(MVP, 1, GL_FALSE, (const GLfloat*) mvp);
    glUniform1f(iTime, glfwGetTime() - time_correction);
  }
};

const char* DefaultShader::vs = R"(
#version 330 core
layout(location = 0) in vec4 vPos;
layout(location = 1) in vec4 vNormal;
out vec4 fColor;
uniform mat4 MVP; 
uniform float iTime;
void main() {
   vec4 lightPos = MVP * vec4(0, 0, 2, 1);
   vec4 lightVec = lightPos - vPos;
   vec4 lightDir = normalize(lightVec); 
   float lightDis = dot(lightVec, lightVec);
   vec4 normal = MVP * vNormal;
   float theta = max(dot(normal, lightDir),0);
   gl_Position =  vec4((MVP * vPos).xyz, 1);
   fColor = 5 * vec4(1, cos(iTime), -sin(iTime), 1) * theta / (lightDis * lightDis);
   fColor += vec4(0.1f);
})";

const char* DefaultShader::fs = R"(
#version 330 core
in vec4 fColor;
out vec4 color;
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
    // Copy source data to the shader data
    glCopyNamedBufferSubData(source, input_buffer, 0, 0, buf_size);

    // Match buffer objects to shader mounts
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, input_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, output_buffer);

    glUseProgram(program); 
    int job_count = buf_size / (sizeof(vec3) * 3);

    glDispatchCompute(job_count / WORK_GROUP_SIZE, 1, 1);
    
    // Ensure all writes of previous shaders have completed
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Copy result back to output buffer
    glCopyNamedBufferSubData(output_buffer, target, 0, 0, buf_size);

    // Mount the result buffer and print it
    glBindBuffer(GL_ARRAY_BUFFER, output_buffer);
  }
};

const char* ComputeShader::src = R"(
# version 430 core
layout(local_size_x = 8, local_size_y = 1) in;
layout(std430, binding=4) buffer inBuf
{
  vec3 vertices[];
};

layout(std430, binding=5) buffer outBuf
{
  vec3 normals[];
};

void main() {
  uint pos = gl_GlobalInvocationID.x * 3;
  vec3 p1 = vertices[pos + 0];
  vec3 p2 = vertices[pos + 1];
  vec3 p3 = vertices[pos + 2];

  vec3 v1 = p1 - p2;
  vec3 v2 = p3 - p2;

  vec3 n = normalize(cross(v1, v2));
  normals[pos + 0] = n; 
  normals[pos + 1] = n; 
  normals[pos + 2] = n; 
}
)";

struct TextureComputeShader {
  static const char* src;
  GLuint src_tex;
  GLuint tex;
  GLuint shader;
  GLuint program;
  GLuint w;
  GLuint h;
  GLint iTime;

  void Init(GLuint src_tex, int w, int h) {
    this->src_tex = src_tex;
    this->w = w;
    this->h = h;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

    //glBindImageTexture(0, src_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    //glBindImageTexture(1, src_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    shader = CompileShader(GL_COMPUTE_SHADER, &src);
    program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);


    iTime = glGetUniformLocation(program, "iTime");
  }

  void Run(float time) {
    glUseProgram(program);
    glUniform1f(iTime, time);
    glDispatchCompute(w, h, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
};

const char* TextureComputeShader::src = R"(
  #version 450 core
  layout(local_size_x = 1, local_size_y = 1) in;
  layout(rgba32f, binding = 0) uniform image2D img_input;
  layout(rgba32f, binding = 1) uniform image2D img_output;
  uniform float iTime;

  uint wang_hash(uint seed)
  {
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
  }


  float uintToFloat(uint seed) {
    return seed * (1.0f / 4294967296.0f);
  }

  float randomf(uint seed) {
    return uintToFloat(wang_hash(seed));
  }

  void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    float r = randomf(coords.x + coords.y * coords.y * 512 * uint(iTime * 1000 * iTime));
    vec4 pixel = imageLoad(img_input, coords);
    vec4 rv = vec4(r, r, r, 1);
    imageStore(img_output, coords, pixel + r);
  }
)";
