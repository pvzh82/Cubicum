#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "../core/shader.h"
#include "chunk.h"
#include "perlinNoise.h"

class World {
 public:
  World();
  ~World();
  void Update(float camX, float camY, float camZ, unsigned int modelLoc);
  void Render(Shader& shader);

 private:
  std::vector<unsigned int> GenerateChunkData(int chunkX,
                                              int chunkY,
                                              int chunkZ);

  std::string GetChunkKey(int x, int y, int z) const;

  std::unordered_map<std::string, std::unique_ptr<Chunk>> chunks;
  unsigned int chunkSize;
  int renderDistance;

  uint32_t worldSeed = 0x12345678;
};
