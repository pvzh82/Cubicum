cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

project(test)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)


# Definir los archivos fuente
set(SOURCE_FILES
    src/core/main.cpp
    src/core/path_manager.cpp
    src/core/shader.cpp
    include/glad/glad.c
)

# Crear el ejecutable
add_executable(test ${SOURCE_FILES})

# Incluir directorios de cabeceras
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(test PRIVATE glfw OpenGL::GL)

set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)