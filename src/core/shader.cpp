#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

#include "shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>


shader::shader(const char* vertexSource, const char* fragmentSource) 
{
    //GET SOURCE CODE FROM FILE

    std::string vertexCode; 
    std::string fragmentCode;
    std::ifstream vertexFile;
    std::ifstream fragmentFile;

    try
    {
        //open files
        vertexFile.open(vertexSource);
        fragmentFile.open(fragmentSource);
        
        std::stringstream vShaderStream, fShaderStream;
        //put all file content into a string
        vShaderStream << vertexFile.rdbuf();
        fShaderStream << fragmentFile.rdbuf();		
        // close
        vertexFile.close();
        fragmentFile.close();
        // & convert
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        
        
    
    }
    //errrors?
    catch(const std::ios_base::failure& e)
    {
        std::cerr <<  "error reading shader files :c" << std::endl << e.what() << std::endl;
    
    }
    //convert into c string
    const char* cVertexCode = vertexCode.c_str();
    const char* cFragmentCode = fragmentCode.c_str();


    //COMPILE SHADERS
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
        std::cerr << "Error compiling vertex shader :c" << std::endl << infoLog << std::endl;
        
    }
    
    // fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &cFragmentCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "Error compiling fragment shader :c" << std::endl << infoLog << std::endl;
        
    }
    
    // create shader program
    shaderID = glCreateProgram();
    glAttachShader(shaderID, vertex);
    glAttachShader(shaderID, fragment);
    glLinkProgram(shaderID);

    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderID, 512, NULL, infoLog);
        std::cerr << "Error with linking program :c" << std::endl << infoLog << std::endl;
    }
    
    // clean up :)
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void shader::useShader() 
{
    glUseProgram(shaderID);
}


void shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}

void shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

void shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}