#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdint>
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

  // Rebuilds mesh using neighbor chunk data for correct border face culling.
  // Pass nullptr for neighbors that don't exist (treated as air).
  void GenerateChunkMesh(
      const std::vector<unsigned int>* negX = nullptr,
      const std::vector<unsigned int>* posX = nullptr,
      const std::vector<unsigned int>* negZ = nullptr,
      const std::vector<unsigned int>* posZ = nullptr);

  void RebuildMesh(
      const std::vector<unsigned int>* negX,
      const std::vector<unsigned int>* posX,
      const std::vector<unsigned int>* negZ,
      const std::vector<unsigned int>* posZ);

  void SetupBuffers();
  void AddFace(int x, int y, int z,
               glm::vec3 normal,
               unsigned int& indexOffset,
               int atlasCol, int atlasRow);

  const std::vector<unsigned int>& getData() const { return chunkData; }

  glm::vec3 position;

 private:
  unsigned int chunkWidth;
  unsigned int chunkHeight;
  std::vector<std::vector<std::vector<uint8_t>>> blocks;
  std::vector<unsigned int> chunkData;
  GLuint vao = 0, vbo = 0, ebo = 0;
  unsigned int numIndices = 0;

  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};
