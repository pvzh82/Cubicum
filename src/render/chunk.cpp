#include "chunk.h"

#include <GLFW/glfw3.h>
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
                std::vector<std::vector<bool>>(
                    chunkHeight, std::vector<bool>(chunkWidth, false)));

  for (int x = 0; x < chunkWidth; x++) {
    for (int y = 0; y < chunkHeight; y++) {
      for (int z = 0; z < chunkWidth; z++) {
        int index = x + y * chunkWidth + z * chunkWidth * chunkHeight;
        blocks[x][y][z] = (chunkData[index] != 0);
      }
    }
  }
  GenerateChunkTerrain();
}

Chunk::~Chunk() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}

void Chunk::GenerateChunkTerrain() {
  auto start = std::chrono::high_resolution_clock::now();
  for (int x = 0; x < chunkWidth; x++) {
    for (int y = 0; y < chunkHeight; y++) {
      for (int z = 0; z < chunkWidth; z++) {
        blocks[x][y][z] = true;
      }
    }
  }
  GenerateChunkMesh();
  SetupBuffers();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Chunk creation time: " << duration.count() << "[ms]" << "\n";
}

void Chunk::GenerateChunkMesh() {
  vertices.clear();
  indices.clear();
  unsigned int indexOffset = 0;
  for (int x = 0; x < chunkWidth; x++) {
    for (int y = 0; y < chunkHeight; y++) {
      for (int z = 0; z < chunkWidth; z++) {
        if (!blocks[x][y][z])
          continue;
        // is block surrounded by more blocks?
        bool top = (y == chunkHeight - 1) || !blocks[x][y + 1][z];
        bool bottom = (y == 0) || !blocks[x][y - 1][z];
        bool front = (z == chunkWidth - 1) || !blocks[x][y][z + 1];
        bool back = (z == 0) || !blocks[x][y][z - 1];
        bool left = (x == chunkWidth - 1) || !blocks[x + 1][y][z];
        bool right = (x == 0) || !blocks[x - 1][y][z];

        if (!top && !bottom && !front && !back && !left && !right) {
          continue;
        }
        // add visible faces
        if (top)
          AddFace(x, y, z, glm::vec3(0, 1, 0), indexOffset);
        if (bottom)
          AddFace(x, y, z, glm::vec3(0, -1, 0), indexOffset);
        if (front)
          AddFace(x, y, z, glm::vec3(0, 0, 1), indexOffset);
        if (back)
          AddFace(x, y, z, glm::vec3(0, 0, -1), indexOffset);
        if (left)
          AddFace(x + 1, y, z, glm::vec3(-1, 0, 0), indexOffset);
        if (right)
          AddFace(x - 1, y, z, glm::vec3(1, 0, 0), indexOffset);
      }
    }
  }
  numIndices = indices.size();
}

void Chunk::AddFace(int x,
                    int y,
                    int z,
                    glm::vec3 normal,
                    unsigned int& indexOffset) {
  float size = 1.0f;  // block size
  // face positions
  glm::vec3 v1(x - size / 2, y - size / 2, z - size / 2);
  glm::vec3 v2(x + size / 2, y - size / 2, z - size / 2);
  glm::vec3 v3(x + size / 2, y + size / 2, z - size / 2);
  glm::vec3 v4(x - size / 2, y + size / 2, z - size / 2);

  if (normal == glm::vec3(0, 1, 0)) {
    v1 = glm::vec3(x, y + 1, z);
    v2 = glm::vec3(x + 1, y + 1, z);
    v3 = glm::vec3(x + 1, y + 1, z + 1);
    v4 = glm::vec3(x, y + 1, z + 1);

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal.x, normal.y,
                                     normal.z, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal.x, normal.y,
                                     normal.z, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal.x, normal.y,
                                     normal.z, 1.0f, 1.0f});
    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z, normal.x, normal.y,
                                     normal.z, 0.0f, 1.0f});
  }
  if (normal == glm::vec3(0, -1, 0)) {
    v1 = glm::vec3(x, y, z);
    v2 = glm::vec3(x + 1, y, z);
    v3 = glm::vec3(x + 1, y, z + 1);
    v4 = glm::vec3(x, y, z + 1);

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal.x, normal.y,
                                     normal.z, 0.0f, 1.0f});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal.x, normal.y,
                                     normal.z, 1.0f, 1.0f});
    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal.x, normal.y,
                                     normal.z, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z, normal.x, normal.y,
                                     normal.z, 0.0f, 0.0f});
  }
  if (normal == glm::vec3(0, 0, 1)) {
    v1 = glm::vec3(x + 1, y, z + 1);
    v2 = glm::vec3(x, y, z + 1);
    v3 = glm::vec3(x, y + 1, z + 1);
    v4 = glm::vec3(x + 1, y + 1, z + 1);

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal.x, normal.y,
                                     normal.z, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal.x, normal.y,
                                     normal.z, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal.x, normal.y,
                                     normal.z, 1.0f, 1.0f});
    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z, normal.x, normal.y,
                                     normal.z, 0.0f, 1.0f});
  }
  if (normal == glm::vec3(0, 0, -1)) {
    v1 = glm::vec3(x, y, z);
    v2 = glm::vec3(x + 1, y, z);
    v3 = glm::vec3(x + 1, y + 1, z);
    v4 = glm::vec3(x, y + 1, z);

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal.x, normal.y,
                                     normal.z, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal.x, normal.y,
                                     normal.z, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal.x, normal.y,
                                     normal.z, 1.0f, 1.0f});
    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z, normal.x, normal.y,
                                     normal.z, 0.0f, 1.0f});
  }
  if (normal == glm::vec3(-1, 0, 0)) {
    v1 = glm::vec3(x, y, z);
    v2 = glm::vec3(x, y, z + 1);
    v3 = glm::vec3(x, y + 1, z + 1);
    v4 = glm::vec3(x, y + 1, z);

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal.x, normal.y,
                                     normal.z, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal.x, normal.y,
                                     normal.z, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal.x, normal.y,
                                     normal.z, 1.0f, 1.0f});
    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z, normal.x, normal.y,
                                     normal.z, 0.0f, 1.0f});
  }
  if (normal == glm::vec3(1, 0, 0)) {
    v1 = glm::vec3(x + 1, y, z + 1);
    v2 = glm::vec3(x + 1, y, z);
    v3 = glm::vec3(x + 1, y + 1, z);
    v4 = glm::vec3(x + 1, y + 1, z + 1);

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z, normal.x, normal.y,
                                     normal.z, 0.0f, 0.0f});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z, normal.x, normal.y,
                                     normal.z, 1.0f, 0.0f});
    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z, normal.x, normal.y,
                                     normal.z, 1.0f, 1.0f});
    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z, normal.x, normal.y,
                                     normal.z, 0.0f, 1.0f});
  }

  indices.push_back(indexOffset);
  indices.push_back(indexOffset + 1);
  indices.push_back(indexOffset + 2);
  indices.push_back(indexOffset + 2);
  indices.push_back(indexOffset + 3);
  indices.push_back(indexOffset);

  indexOffset += 4;
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
