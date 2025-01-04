#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <glm/glm.hpp>
#include <filesystem>


class shader {
public:
    unsigned int shaderID; 

    shader(const char* vertexSource, const char* fragmentSource);

    void useShader();
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

    
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
};


#endif