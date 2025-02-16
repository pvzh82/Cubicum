#pragma once
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <vector>
class Chunk {
 public:
  Chunk(unsigned int chunkSize);
  ~Chunk();

  void GenerateChunkTerrain();
  void GenerateChunkMesh();
  void Render();

  void SetupBuffers();

  void AddFace(int x, int y, int z,glm::vec3 normal, unsigned int &indexOffset);

 private:
  unsigned int chunkSize;
  std::vector<std::vector<std::vector<bool>>> blocks;

  std::vector<float> vertices;
  std::vector<unsigned int> indices;


  GLuint vao, vbo, ebo;
  unsigned int numIndices;
};