#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Chunk {
 public:
  Chunk(unsigned int chunkWidth,
        unsigned int chunkHeight,
        const std::vector<unsigned int>& chunkData,
        const glm::vec3& position);
  ~Chunk();

  void Render(const glm::mat4& modelMatrix);

  void GenerateChunkTerrain();
  void GenerateChunkMesh();
  void Render();
  void SetupBuffers();
  void AddFace(int x,
               int y,
               int z,
               glm::vec3 normal,
               unsigned int& indexOffset);

  glm::vec3 position;

 private:
  unsigned int chunkWidth;
  unsigned int chunkHeight;
  std::vector<std::vector<std::vector<bool>>> blocks;
  std::vector<unsigned int> chunkData;
  GLuint vao, vbo, ebo;
  unsigned int numIndices;

  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};
