#include <stdexcept>

struct VertexBuffer {
  GLuint buffer;
  size_t buf_size;
  GLuint vao;
  bool initialized = false;

  VertexBuffer() {}
  void Init(float* vertices_begin, float* vertices_end) {
    buf_size = vertices_end - vertices_begin;
    glGenBuffers(1, &buffer); 
    glGenVertexArrays(1, &vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, buf_size * sizeof(float), vertices_begin, GL_STATIC_DRAW);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, // elements per vertex
        GL_FLOAT,
        GL_FALSE, // normalized
        0,
        (void*)(sizeof(float) * 0));
    initialized = true;
  }

  void Draw() {
    if (!initialized)
      throw std::invalid_argument("Initialize vertex buffer first");
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, this->buf_size);
  }
};
