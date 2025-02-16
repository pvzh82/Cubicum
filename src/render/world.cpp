#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../core/shader.h"
#include "chunk.h"
#include "perlinNoise.h"
#include "world.h"

World::World() : chunkSize(16), renderDistance(4) {
  for (int x = -renderDistance; x <= renderDistance; x++) {
    for (int y = -renderDistance; y <= renderDistance; y++) {
      for (int z = -renderDistance; z <= renderDistance; z++) {
        std::string chunkString = std::to_string(x) + "," + std::to_string(y) +
                                  "," + std::to_string(z);
        chunks[chunkString] =
            std::make_unique<Chunk>(chunkSize, 64, GenerateChunkData(x, y, z),
                                    glm::vec3(x * chunkSize, y, z * chunkSize));
      }
    }
  }
}

World::~World() {}

std::vector<unsigned int> World::GenerateChunkData(int chunkX,
                                                   int chunkY,
                                                   int chunkZ) {
  std::vector<unsigned int> data;
  data.reserve(chunkSize * chunkSize * chunkSize);

  float noiseScale = 0.02f;
  float amplitude = 30.0f;

  for (int x = 0; x < chunkSize; x++) {
    for (int z = 0; z < chunkSize; z++) {
      float worldX = chunkX * chunkSize + x;
      float worldZ = chunkZ * chunkSize + z;

      float height =
          amplitude * (0.5f + PerlinNoise::noise(worldX * noiseScale,
                                                 worldZ * noiseScale, 0.0f));

      // just 4 testing
      // En World::GenerateChunkData
      float noiseVal =
          PerlinNoise::noise(worldX * noiseScale, worldZ * noiseScale, 0.0f);
      std::cout << "Noise value at (" << worldX << "," << worldZ
                << "): " << noiseVal << std::endl;

      for (int y = 0; y < chunkSize; y++) {
        float worldY = chunkY * chunkSize + y;

        unsigned int blockType = 0;
        if (worldY <= height) {
          if (worldY > height - 1)
            blockType = 1;  // grass
          else if (worldY > height - 5)
            blockType = 2;  // dirt
          else
            blockType = 3;  // stone
        }
        data.push_back(blockType);
        if (blockType != 0) {
          std::cout << "Block at (" << x << "," << y << "," << z
                    << ") = " << blockType << "\n";
        }
      }
    }
  }
  return data;
}

void World::Render(Shader& shader) {
  for (auto& [key, chunk] : chunks) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), chunk->position);
    shader.setMat4("model", model);
    chunk->Render(model);
  }
}

void World::Update(float camX, float camY, float camZ, unsigned int modelLoc) {
  int currentChunkX = static_cast<int>(std::floor(camX / chunkSize));
  int currentChunkZ = static_cast<int>(std::floor(camZ / chunkSize));

  for (int x = currentChunkX - renderDistance;
       x <= currentChunkX + renderDistance; x++) {
    for (int z = currentChunkZ - renderDistance;
         z <= currentChunkZ + renderDistance; z++) {
      std::string key = GetChunkKey(x, 0, z);

      if (!chunks.count(key)) {
        auto chunkData = GenerateChunkData(x, 0, z);
        glm::vec3 position(x * chunkSize, 0, z * chunkSize);
        chunks[key] =
            std::make_unique<Chunk>(chunkSize, 64, chunkData, position);
      }
    }
  }

  auto it = chunks.begin();
  while (it != chunks.end()) {
    std::string key = it->first;
    size_t firstComma = key.find(',');
    size_t lastComma = key.rfind(',');
    int chunkX = std::stoi(key.substr(0, firstComma));
    int chunkZ = std::stoi(key.substr(lastComma + 1));

    if (abs(chunkX - currentChunkX) > renderDistance ||
        abs(chunkZ - currentChunkZ) > renderDistance) {
      it = chunks.erase(it);
    } else {
      ++it;
    }
  }
}

std::string World::GetChunkKey(int x, int y, int z) const {
  return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
}
