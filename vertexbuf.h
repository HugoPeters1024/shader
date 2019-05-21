#include <stdexcept>

struct VertexMesh {
  uint w, h;
  VertexMesh() {}
  size_t vertexCount;
  GLuint vertexBuffer;
  GLuint normalBuffer;
  GLuint vao;
  ComputeShader worker;
  DefaultShader shader;

  void Draw(mat4x4 mvp, float time_correction) {
    worker.Run();
    shader.Bind(mvp, time_correction);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
  }

  void Init(uint w, uint h) {
    shader.Init();
    vertexCount = w * h * 24;
    float* grid = (float*)malloc(sizeof(float) * vertexCount);
    float* normals = (float*)malloc(sizeof(float) * vertexCount);
    float* height = (float*)malloc(sizeof(float) * (w+1) * (h+1));
    for(int i=0; i<(w+1)*(h+1); i++) {
      height[i] = static_cast<float> (rand()) / static_cast<float>(RAND_MAX) * 0.1f;
    }
    int q = 0;
    float scalex = (float)w / 2.0f;
    float scaley = (float)h / 2.0f;
    for(int y=0; y<h; y++)
      for(int x=0; x<w; x++) {
        grid[q+0] = x/scalex - 1;
        grid[q+1] = y/scaley - 1;
        grid[q+2] = height[x + y * h];
        grid[q+3] = 1;

        grid[q+4] = x/scalex - 1;
        grid[q+5] = (y+1)/scaley - 1;
        grid[q+6] = height[x + (y+1) * h];
        grid[q+7] = 1;

        grid[q+8] = (x+1)/scalex - 1;
        grid[q+9] = y/scaley- 1;
        grid[q+10] = height[(x+1) + y * h];
        grid[q+11] = 1;

        grid[q+12] = (x+1)/scalex - 1;
        grid[q+13] = y/scaley - 1;
        grid[q+14] = height[(x+1) + y * h];
        grid[q+15] = 1;

        grid[q+16] = x/scalex - 1;
        grid[q+17] = (y+1)/scaley - 1;
        grid[q+18] = height[x + (y+1) * h];
        grid[q+19] = 1;

        grid[q+20] = (x+1)/scalex - 1;
        grid[q+21] = (y+1)/scaley - 1;
        grid[q+22] = height[(x+1) + (y+1) * h];
        grid[q+23] = 1;
        q+=24;
      }

    for(int i=0; i<vertexCount; i+=12) {
      vec3 p1 = { grid[i+0], grid[i+1], grid[i+2] }; 
      vec3 p2 = { grid[i+4], grid[i+5], grid[i+6] }; 
      vec3 p3 = { grid[i+8], grid[i+9], grid[i+10] }; 

      vec3 v1, v2;
      vec3_sub(v1, p1, p2);
      vec3_sub(v2, p3, p2);

      vec3 cross, normal;
      vec3_mul_cross(cross, v1, v2);
      vec3_norm(normal, cross);

      normals[i+0] = normal[0];
      normals[i+1] = normal[1];
      normals[i+2] = normal[2];
      normals[i+3] = 0;

      normals[i+4] = normal[0];
      normals[i+5] = normal[1];
      normals[i+6] = normal[2];
      normals[i+7] = 0;

      normals[i+8] = normal[0];
      normals[i+9] = normal[1];
      normals[i+10] = normal[2];
      normals[i+11] = 0;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertexBuffer); 
    glGenBuffers(1, &normalBuffer); 

    glBindVertexArray(vao);

    // Activate attributes 0 and 1
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Mount the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    //Send vertices to GPU
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), grid, GL_STATIC_DRAW);
    // Connect the mounted vertex buffer to attribute 0
    glVertexAttribPointer(
        shader.vPos, //atrib index
        4, // elements per vertex
        GL_FLOAT,
        GL_FALSE, // normalized
        0, // bytes padding per vertex
        (void*)(sizeof(float) * 0));
    
    // Mount thhe normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    // Send vertices to GPU
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), normals, GL_STATIC_DRAW);
    // Connect the mounted normal buffer to attribute 1
    glVertexAttribPointer(
        shader.vNormal, //atrib index
        4, // elements per normal
        GL_FLOAT,
        GL_TRUE, // normalized
        0, // bytes padding per normal
        (void*)(sizeof(float) * 0));

    free(grid);
    free(normals);
    free(height);

    worker.Init(vertexBuffer, normalBuffer, vertexCount * sizeof(float));
  }
};
