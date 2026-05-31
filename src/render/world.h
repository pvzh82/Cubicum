#ifndef WORLD_H
#define WORLD_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <cstring>
#include "../core/shader.h"
#include "chunk.h"

struct TupleHash {
  std::size_t operator()(const std::tuple<int, int, int>& key) const {
    auto h1 = std::hash<int>()(std::get<0>(key));
    auto h2 = std::hash<int>()(std::get<1>(key));
    auto h3 = std::hash<int>()(std::get<2>(key));
    std::size_t seed = h1;
    seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
  }
};

class World {
 public:
  World();
  ~World();

  void Render(Shader& shader);
  void Update(float camX, float camY, float camZ, unsigned int modelLoc);

 private:
  std::vector<unsigned int> GenerateChunkData(int chunkX,
                                              int chunkY,
                                              int chunkZ);

  int chunkSize;
  int chunkHeight;
  int renderDistance;

  // Heightmap data
  unsigned char* heightmapData = nullptr;
  int heightmapWidth = 0;
  int heightmapHeight = 0;
  const float HEIGHT_SCALE = 60.0f;  // Pixel [0,255] maps to height [0,60]

  int heightmapMin = 0;
  int heightmapMax = 255;

  bool LoadHeightmap(const char* path);
  float SampleHeightmap(float worldX, float worldZ);
  const std::vector<unsigned int>* getNeighborData(int cx, int cz) const;
  void rebuildWithNeighbors(int cx, int cz);

  std::unordered_map<std::tuple<int, int, int>,
                     std::unique_ptr<Chunk>,
                     TupleHash>
      chunks;
};

#endif  // WORLD_H
