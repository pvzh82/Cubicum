#ifndef PATHMANAGER_H
#define PATHMANAGER_H

#include <filesystem>
#include <string>

class PathManager {
 public:
  static std::string getShaderPath(const std::string& shaderName);
  static std::string getTexturePath(const std::string& textureName);

  static const std::filesystem::path assetPath;
};

#endif
