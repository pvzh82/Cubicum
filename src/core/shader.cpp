#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
  // GET SOURCE CODE FROM FILE

  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vertexFile;
  std::ifstream fragmentFile;

  try {
    // open files
    vertexFile.open(vertexSource);
    fragmentFile.open(fragmentSource);

    std::stringstream vShaderStream, fShaderStream;
    // put all file content into a string
    vShaderStream << vertexFile.rdbuf();
    fShaderStream << fragmentFile.rdbuf();
    // close
    vertexFile.close();
    fragmentFile.close();
    // & convert
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

  }
  // errrors?
  catch (const std::ios_base::failure& e) {
    std::cerr << "error reading shader files :c" << std::endl
              << e.what() << std::endl;
  }
  // convert into c string
  const char* cVertexCode = vertexCode.c_str();
  const char* cFragmentCode = fragmentCode.c_str();

  // COMPILE SHADERS
  unsigned int vertex, fragment;
  int success;
  char infoLog[512];

  // vertex
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &cVertexCode, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    std::cerr << "Error compiling vertex shader :c" << std::endl
              << infoLog << std::endl;
  }

  // fragment
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &cFragmentCode, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    std::cerr << "Error compiling fragment shader :c" << std::endl
              << infoLog << std::endl;
  }

  // create shader program
  shaderID = glCreateProgram();
  glAttachShader(shaderID, vertex);
  glAttachShader(shaderID, fragment);
  glLinkProgram(shaderID);

  glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderID, 512, NULL, infoLog);
    std::cerr << "Error with linking program :c" << std::endl
              << infoLog << std::endl;
  }

  // clean up :)
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

void Shader::useShader() {
  glUseProgram(shaderID);
}

void Shader::setBool(const std::string& name, bool value) const {
  glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
  glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
  glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
  glUniform2fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const {
  glUniform2f(glGetUniformLocation(shaderID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
  glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const {
  glUniform3f(glGetUniformLocation(shaderID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
  glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name,
                     float x,
                     float y,
                     float z,
                     float w) const {
  glUniform4f(glGetUniformLocation(shaderID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const {
  glUniformMatrix2fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
  glUniformMatrix3fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
  glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE,
                     &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << std::endl
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << std::endl
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
  }
}