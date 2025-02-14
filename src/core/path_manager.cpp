#include "path_manager.h"
#include <iostream>

const std::filesystem::path PathManager::assetPath = "../assets";


std::string PathManager::getShaderPath(const std::string& shaderName) {
    std::filesystem::path shaderPath = (assetPath / "shaders" / shaderName);


    if (!std::filesystem::exists(shaderPath)) {
        std::cerr << "shader not found at: " << shaderPath << std::endl;
        exit(1);
    }

    return shaderPath.string();

}


std::string PathManager::getTexturePath(const std::string& textureName) {
    std::filesystem::path texturePath = assetPath / "textures" / textureName;

    if (!std::filesystem::exists(texturePath)) {
        std::cerr << "texture not found at: " << texturePath << std::endl;
        exit(1);
    }

    return texturePath.string();
}