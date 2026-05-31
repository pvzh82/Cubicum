#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iomanip>
#include <iostream>
#include <memory>
#include <unordered_map>

#include <cmath>
#include <cstring>
#include <tuple>
#include <vector>

#include "../core/shader.h"
#include "chunk.h"
#include "perlinNoise.h"
#include "stb_image/stb_image.h"
#include "world.h"

World::World() : chunkSize(16), chunkHeight(96), renderDistance(6) {
  // Procedural generation active — heightmap loading disabled
  // if (!LoadHeightmap("../assets/heightmaps/terrain.png")) {
  //   std::cerr << "Warning: Could not load heightmap, using procedural generation\n";
  // }

  std::cout << "Creating chunks with renderDistance=" << renderDistance << "\n";
  int chunkCount = 0;
  int minCoord = -static_cast<int>(renderDistance);
  int maxCoord = static_cast<int>(renderDistance);

  for (int x = minCoord; x <= maxCoord; x++) {
    for (int z = minCoord; z <= maxCoord; z++) {
      int y = 0;
      std::tuple<int, int, int> chunkKey = std::make_tuple(x, y, z);
      std::vector<unsigned int> chunkData = GenerateChunkData(x, y, z);
      glm::vec3 position(x * chunkSize, y * chunkHeight, z * chunkSize);
      chunks[chunkKey] =
          std::make_unique<Chunk>(chunkSize, chunkHeight, chunkData, position);
      chunkCount++;
    }
  }
  std::cout << "Created " << chunkCount << " chunks\n";

  // Rebuild all meshes now that all neighbors exist.
  for (int x = minCoord; x <= maxCoord; x++) {
    for (int z = minCoord; z <= maxCoord; z++) {
      rebuildWithNeighbors(x, z);
    }
  }
}

World::~World() {
  if (heightmapData) {
    stbi_image_free(heightmapData);
  }
}

std::vector<unsigned int> World::GenerateChunkData(int chunkX,
                                                   int chunkY,
                                                   int chunkZ) {
  std::vector<unsigned int> data;
  data.reserve(chunkSize * chunkSize * chunkHeight);

  static bool debugPrinted = false;
  int blockCount = 0;
  float minHeight = 999.0f, maxHeight = -999.0f;

  // Layout matches Chunk index formula: x + y*W + z*W*H → iterate z outer, y mid, x inner
  for (int z = 0; z < chunkSize; z++) {
    for (int y = 0; y < chunkHeight; y++) {
      for (int x = 0; x < chunkSize; x++) {
        float worldX = chunkX * chunkSize + x;
        float worldZ = chunkZ * chunkSize + z;
        float worldY = chunkY * chunkHeight + y;

        float height;
        if (heightmapData) {
          height = SampleHeightmap(worldX, worldZ);
          if (height < minHeight)
            minHeight = height;
          if (height > maxHeight)
            maxHeight = height;
        } else {
          static PerlinNoise perlin;
          float h = 0.0f;
          float freq = 0.005f;
          float amp  = 1.0f;
          float maxAmp = 0.0f;
          for (int octave = 0; octave < 7; ++octave) {
            h += amp * perlin.noise(worldX * freq, worldZ * freq, 0.0f);
            maxAmp += amp;
            freq *= 2.0f;
            amp  *= 0.4f;
          }
          h /= maxAmp;                   // normalize to [-1, 1]
          h = (h + 1.0f) * 0.5f;        // remap to [0, 1]
          // Plains below 0.4, hills/mountains above — tweak first value to taste
          h = glm::smoothstep(0.35f, 0.75f, h);
          height = 8.0f + h * 52.0f;
          height = glm::clamp(height, 0.0f, static_cast<float>(chunkHeight - 1));
        }

        unsigned int blockType = 0;
        if (worldY <= height) {
          if (worldY > height - 1)
            blockType = 1;  // grass
          else if (worldY > height - 5)
            blockType = 2;  // dirt
          else
            blockType = 3;  // stone
          blockCount++;
        }
        data.push_back(blockType);
      }
    }
  }

  if (!debugPrinted && chunkX == 0 && chunkZ == 0) {
    std::cout << "DEBUG Chunk(0,0,0):\n";
    std::cout << "  Blocks generated: " << blockCount << "\n";
    std::cout << "  Height range: [" << minHeight << ", " << maxHeight << "]\n";
    std::cout << "  Sample 4x4 heights:\n";
    for (int sx = 0; sx < 4; sx++) {
      for (int sz = 0; sz < 4; sz++) {
        float h =
            SampleHeightmap(static_cast<float>(sx), static_cast<float>(sz));
        std::cout << h << (sz == 3 ? "\n" : " ");
      }
    }
    debugPrinted = true;
  }

  return data;
}

bool World::LoadHeightmap(const char* path) {
  int channels;
  unsigned char* data =
      stbi_load(path, &heightmapWidth, &heightmapHeight, &channels, 1);

  if (!data) {
    std::cerr << "Failed to load heightmap: " << path << "\n";
    return false;
  }

  heightmapData = data;

  // Debug: Check min/max values
  unsigned char minVal = 255, maxVal = 0;
  for (int i = 0; i < heightmapWidth * heightmapHeight; ++i) {
    if (heightmapData[i] < minVal)
      minVal = heightmapData[i];
    if (heightmapData[i] > maxVal)
      maxVal = heightmapData[i];
  }
  heightmapMin = static_cast<int>(minVal);
  heightmapMax = static_cast<int>(maxVal);

  std::cout << "Loaded heightmap: " << path << " (" << heightmapWidth << "x"
            << heightmapHeight << ")\n";
  std::cout << "  PNG values: min=" << (int)minVal << " max=" << (int)maxVal
            << "\n";
  return true;
}

float World::SampleHeightmap(float worldX, float worldZ) {
  if (!heightmapData || heightmapWidth == 0 || heightmapHeight == 0) {
    return 0.0f;
  }

  // Muestreo directo del heightmap: cada pixel ~1 bloque
  const float PIXEL_SCALE = 1.0f;
  float u = worldX / PIXEL_SCALE;
  float v = worldZ / PIXEL_SCALE;

  // Wrap coordinates to repeat the heightmap seamlessly
  u = std::fmod(u, static_cast<float>(heightmapWidth));
  v = std::fmod(v, static_cast<float>(heightmapHeight));

  if (u < 0)
    u += heightmapWidth;
  if (v < 0)
    v += heightmapHeight;

  // Bilinear interpolation
  int x0 = static_cast<int>(u);
  int z0 = static_cast<int>(v);
  int x1 = (x0 + 1) % heightmapWidth;
  int z1 = (z0 + 1) % heightmapHeight;

  float fx = u - x0;  // fractional part
  float fz = v - z0;

  auto sampleNorm = [&](int px, int pz) {
    unsigned char pv = heightmapData[pz * heightmapWidth + px];
    int range = heightmapMax - heightmapMin;
    float norm = 0.0f;
    if (range > 0)
      norm = (static_cast<int>(pv) - heightmapMin) / static_cast<float>(range);
    return norm * HEIGHT_SCALE;
  };

  float h00 = sampleNorm(x0, z0);
  float h10 = sampleNorm(x1, z0);
  float h01 = sampleNorm(x0, z1);
  float h11 = sampleNorm(x1, z1);

  // Interpolación suavizada con smoothstep para reducir peldaños
  auto smooth = [](float t) { return t * t * (3.0f - 2.0f * t); };
  float sx = smooth(fx);
  float sz = smooth(fz);

  float h0 = h00 * (1.0f - sx) + h10 * sx;
  float h1 = h01 * (1.0f - sx) + h11 * sx;
  float height = h0 * (1.0f - sz) + h1 * sz;

  if (height < 0.0f)
    height = 0.0f;
  if (height > chunkHeight - 1)
    height = static_cast<float>(chunkHeight - 1);

  return height;
}

// Returns the data array of a neighbor chunk, or nullptr if it doesn't exist.
const std::vector<unsigned int>* World::getNeighborData(int cx, int cz) const {
  auto it = chunks.find(std::make_tuple(cx, 0, cz));
  if (it == chunks.end()) return nullptr;
  return &it->second->getData();
}

void World::rebuildWithNeighbors(int cx, int cz) {
  auto it = chunks.find(std::make_tuple(cx, 0, cz));
  if (it == chunks.end()) return;
  it->second->RebuildMesh(
      getNeighborData(cx - 1, cz),
      getNeighborData(cx + 1, cz),
      getNeighborData(cx, cz - 1),
      getNeighborData(cx, cz + 1));
}

void World::Render(Shader& shader) {
  for (auto& [key, chunk] : chunks) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), chunk->position);
    shader.setMat4("model", model);
    chunk->Render(model);
  }
}

void World::Update(float camX, float camY, float camZ, unsigned int modelLoc) {
  int currentChunkX = static_cast<int>(
      std::floor(static_cast<double>(camX) / static_cast<double>(chunkSize)));
  int currentChunkZ = static_cast<int>(
      std::floor(static_cast<double>(camZ) / static_cast<double>(chunkSize)));

  int chunksLoadedThisFrame = 0;
  for (int x = currentChunkX - static_cast<int>(renderDistance);
       x <= currentChunkX + static_cast<int>(renderDistance); x++) {
    for (int z = currentChunkZ - static_cast<int>(renderDistance);
         z <= currentChunkZ + static_cast<int>(renderDistance); z++) {
      std::tuple<int, int, int> key = std::make_tuple(x, 0, z);
      if (chunks.find(key) == chunks.end()) {
        if (chunksLoadedThisFrame >= 1) continue;  // defer to next frame
        auto chunkData = GenerateChunkData(x, 0, z);
        glm::vec3 position(x * chunkSize, 0, z * chunkSize);
        chunks[key] = std::make_unique<Chunk>(chunkSize, chunkHeight, chunkData,
                                              position);
        rebuildWithNeighbors(x, z);
        rebuildWithNeighbors(x - 1, z);
        rebuildWithNeighbors(x + 1, z);
        rebuildWithNeighbors(x, z - 1);
        rebuildWithNeighbors(x, z + 1);
        chunksLoadedThisFrame++;
      }
    }
  }

  // Safe iteration with erase
  for (auto it = chunks.begin(); it != chunks.end();) {
    int chunkX = std::get<0>(it->first);
    int chunkZ = std::get<2>(it->first);
    if (std::abs(chunkX - currentChunkX) > static_cast<int>(renderDistance) ||
        std::abs(chunkZ - currentChunkZ) > static_cast<int>(renderDistance)) {
      it = chunks.erase(it);  // erase returns iterator to next element
    } else {
      ++it;
    }
  }
}
