#include "chunk.h"
#include "texture.h"

#include <GLFW/glfw3.h>
#include <cstdint>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>
#include <vector>

Chunk::Chunk(unsigned int chunkWidth,
             unsigned int chunkHeight,
             const std::vector<unsigned int>& chunkData,
             const glm::vec3& position)
    : chunkWidth(chunkWidth),
      chunkHeight(chunkHeight),
      position(position),
      chunkData(chunkData) {
      blocks.resize(chunkWidth,
              std::vector<std::vector<uint8_t>>(
                  chunkHeight, std::vector<uint8_t>(chunkWidth, 0)));


  GenerateChunkTerrain();
}

Chunk::~Chunk() {
  if (vao != 0)
    glDeleteVertexArrays(1, &vao);
  if (vbo != 0)
    glDeleteBuffers(1, &vbo);
  if (ebo != 0)
    glDeleteBuffers(1, &ebo);
}

void Chunk::GenerateChunkTerrain() {
  auto start = std::chrono::high_resolution_clock::now();
  // Use chunkData from constructor (already populated with heightmap values)
  for (int x = 0; x < chunkWidth; x++) {
    for (int y = 0; y < chunkHeight; y++) {
      for (int z = 0; z < chunkWidth; z++) {
        int index = x + y * chunkWidth + z * chunkWidth * chunkHeight;
        blocks[x][y][z] = static_cast<uint8_t>(chunkData[index]);
      }
    }
  }
  GenerateChunkMesh();  // no neighbors yet on first build
  SetupBuffers();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Chunk creation time: " << duration.count() << "[ms]" << "\n";
}

// Returns whether a block at neighbor-chunk local coords is solid.
// neighborData is the flat data array of the adjacent chunk (may be nullptr).
static bool neighborSolid(const std::vector<unsigned int>* neighborData,
                           int nx, int ny, int nz,
                           int W, int H) {
  if (!neighborData) return false;  // no neighbor → treat as air → show face
  if (ny < 0 || ny >= H) return false;
  int idx = nx + ny * W + nz * W * H;
  return (*neighborData)[idx] != 0;
}

void Chunk::GenerateChunkMesh(
    const std::vector<unsigned int>* negX,
    const std::vector<unsigned int>* posX,
    const std::vector<unsigned int>* negZ,
    const std::vector<unsigned int>* posZ) {
  vertices.clear();
  indices.clear();
  unsigned int indexOffset = 0;

  const int W = static_cast<int>(chunkWidth);
  const int H = static_cast<int>(chunkHeight);

  for (int x = 0; x < W; x++) {
    for (int y = 0; y < H; y++) {
      for (int z = 0; z < W; z++) {
        if (!blocks[x][y][z])
          continue;

        // For border faces, look up the neighboring chunk's block instead of assuming air.
        bool top    = (y == H-1) || !blocks[x][y+1][z];
        bool bottom = (y == 0)   || !blocks[x][y-1][z];

        bool front = (z == W-1) ? !neighborSolid(posZ, x, y, 0,   W, H)
                                : !blocks[x][y][z+1];
        bool back  = (z == 0)   ? !neighborSolid(negZ, x, y, W-1, W, H)
                                : !blocks[x][y][z-1];
        bool left  = (x == 0)   ? !neighborSolid(negX, W-1, y, z, W, H)
                                : !blocks[x-1][y][z];
        bool right = (x == W-1) ? !neighborSolid(posX, 0,   y, z, W, H)
                                : !blocks[x+1][y][z];

        if (!top && !bottom && !front && !back && !left && !right) {
          continue;
        }
        uint8_t blockType = blocks[x][y][z];

        // add visible faces
        if (top) {
          auto [col, row] = getAtlasCell(blockType, true, false);
          AddFace(x, y, z, glm::vec3(0, 1, 0), indexOffset, col, row);
        }
        if (bottom) {
          auto [col, row] = getAtlasCell(blockType, false, true);
          AddFace(x, y, z, glm::vec3(0, -1, 0), indexOffset, col, row);
        }
        if (front) {
          auto [col, row] = getAtlasCell(blockType, false, false);
          AddFace(x, y, z, glm::vec3(0, 0, 1), indexOffset, col, row);
        }
        if (back) {
          auto [col, row] = getAtlasCell(blockType, false, false);
          AddFace(x, y, z, glm::vec3(0, 0, -1), indexOffset, col, row);
        }
        if (left) {
          auto [col, row] = getAtlasCell(blockType, false, false);
          AddFace(x, y, z, glm::vec3(-1, 0, 0), indexOffset, col, row);
        }
        if (right) {
          auto [col, row] = getAtlasCell(blockType, false, false);
          AddFace(x, y, z, glm::vec3(1, 0, 0), indexOffset, col, row);
        }
      }
    }
  }
  numIndices = indices.size();
}

void Chunk::AddFace(int x,
                    int y,
                    int z,
                    glm::vec3 normal,
                    unsigned int& indexOffset,
                    int atlasCol,
                    int atlasRow) {
  // Convert atlas cell (col, row) to UV coordinates in [0,1] space.
  // Each cell occupies 1/ATLAS_SIZE of the texture in each axis.
  const float cs = 1.0f / static_cast<float>(ATLAS_SIZE);
  float u0 = atlasCol * cs;
  float u1 = (atlasCol + 1) * cs;
  // V is flipped because stbi_set_flip_vertically_on_load(true) makes row 0 of PNG → v=1.0
  float v1 = 1.0f - atlasRow * cs;
  float v0 = 1.0f - (atlasRow + 1) * cs;

  glm::vec3 v1p, v2p, v3p, v4p;

  if (normal == glm::vec3(0, 1, 0)) {
    v1p = glm::vec3(x, y + 1, z);
    v2p = glm::vec3(x + 1, y + 1, z);
    v3p = glm::vec3(x + 1, y + 1, z + 1);
    v4p = glm::vec3(x, y + 1, z + 1);

    vertices.insert(vertices.end(), {v1p.x, v1p.y, v1p.z, normal.x, normal.y, normal.z, u0, v0});
    vertices.insert(vertices.end(), {v2p.x, v2p.y, v2p.z, normal.x, normal.y, normal.z, u1, v0});
    vertices.insert(vertices.end(), {v3p.x, v3p.y, v3p.z, normal.x, normal.y, normal.z, u1, v1});
    vertices.insert(vertices.end(), {v4p.x, v4p.y, v4p.z, normal.x, normal.y, normal.z, u0, v1});
  }
  if (normal == glm::vec3(0, -1, 0)) {
    v1p = glm::vec3(x, y, z);
    v2p = glm::vec3(x + 1, y, z);
    v3p = glm::vec3(x + 1, y, z + 1);
    v4p = glm::vec3(x, y, z + 1);

    vertices.insert(vertices.end(), {v1p.x, v1p.y, v1p.z, normal.x, normal.y, normal.z, u0, v1});
    vertices.insert(vertices.end(), {v2p.x, v2p.y, v2p.z, normal.x, normal.y, normal.z, u1, v1});
    vertices.insert(vertices.end(), {v3p.x, v3p.y, v3p.z, normal.x, normal.y, normal.z, u1, v0});
    vertices.insert(vertices.end(), {v4p.x, v4p.y, v4p.z, normal.x, normal.y, normal.z, u0, v0});
  }
  if (normal == glm::vec3(0, 0, 1)) {
    v1p = glm::vec3(x + 1, y, z + 1);
    v2p = glm::vec3(x, y, z + 1);
    v3p = glm::vec3(x, y + 1, z + 1);
    v4p = glm::vec3(x + 1, y + 1, z + 1);

    vertices.insert(vertices.end(), {v1p.x, v1p.y, v1p.z, normal.x, normal.y, normal.z, u0, v0});
    vertices.insert(vertices.end(), {v2p.x, v2p.y, v2p.z, normal.x, normal.y, normal.z, u1, v0});
    vertices.insert(vertices.end(), {v3p.x, v3p.y, v3p.z, normal.x, normal.y, normal.z, u1, v1});
    vertices.insert(vertices.end(), {v4p.x, v4p.y, v4p.z, normal.x, normal.y, normal.z, u0, v1});
  }
  if (normal == glm::vec3(0, 0, -1)) {
    v1p = glm::vec3(x, y, z);
    v2p = glm::vec3(x + 1, y, z);
    v3p = glm::vec3(x + 1, y + 1, z);
    v4p = glm::vec3(x, y + 1, z);

    vertices.insert(vertices.end(), {v1p.x, v1p.y, v1p.z, normal.x, normal.y, normal.z, u0, v0});
    vertices.insert(vertices.end(), {v2p.x, v2p.y, v2p.z, normal.x, normal.y, normal.z, u1, v0});
    vertices.insert(vertices.end(), {v3p.x, v3p.y, v3p.z, normal.x, normal.y, normal.z, u1, v1});
    vertices.insert(vertices.end(), {v4p.x, v4p.y, v4p.z, normal.x, normal.y, normal.z, u0, v1});
  }
  if (normal == glm::vec3(-1, 0, 0)) {
    v1p = glm::vec3(x, y, z);
    v2p = glm::vec3(x, y, z + 1);
    v3p = glm::vec3(x, y + 1, z + 1);
    v4p = glm::vec3(x, y + 1, z);

    vertices.insert(vertices.end(), {v1p.x, v1p.y, v1p.z, normal.x, normal.y, normal.z, u0, v0});
    vertices.insert(vertices.end(), {v2p.x, v2p.y, v2p.z, normal.x, normal.y, normal.z, u1, v0});
    vertices.insert(vertices.end(), {v3p.x, v3p.y, v3p.z, normal.x, normal.y, normal.z, u1, v1});
    vertices.insert(vertices.end(), {v4p.x, v4p.y, v4p.z, normal.x, normal.y, normal.z, u0, v1});
  }
  if (normal == glm::vec3(1, 0, 0)) {
    v1p = glm::vec3(x + 1, y, z + 1);
    v2p = glm::vec3(x + 1, y, z);
    v3p = glm::vec3(x + 1, y + 1, z);
    v4p = glm::vec3(x + 1, y + 1, z + 1);

    vertices.insert(vertices.end(), {v1p.x, v1p.y, v1p.z, normal.x, normal.y, normal.z, u0, v0});
    vertices.insert(vertices.end(), {v2p.x, v2p.y, v2p.z, normal.x, normal.y, normal.z, u1, v0});
    vertices.insert(vertices.end(), {v3p.x, v3p.y, v3p.z, normal.x, normal.y, normal.z, u1, v1});
    vertices.insert(vertices.end(), {v4p.x, v4p.y, v4p.z, normal.x, normal.y, normal.z, u0, v1});
  }

  indices.push_back(indexOffset);
  indices.push_back(indexOffset + 1);
  indices.push_back(indexOffset + 2);
  indices.push_back(indexOffset + 2);
  indices.push_back(indexOffset + 3);
  indices.push_back(indexOffset);

  indexOffset += 4;
}

void Chunk::RebuildMesh(
    const std::vector<unsigned int>* negX,
    const std::vector<unsigned int>* posX,
    const std::vector<unsigned int>* negZ,
    const std::vector<unsigned int>* posZ) {
  GenerateChunkMesh(negX, posX, negZ, posZ);

  // Update existing GPU buffers without allocating new ones.
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Chunk::SetupBuffers() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

void Chunk::Render(const glm::mat4& modelMatrix) {
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
