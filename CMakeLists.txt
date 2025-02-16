cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)


project(test)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

execute_process(COMMAND clear) #clear terminal befor execute

set(SOURCE_FILES
    src/core/main.cpp
    src/core/path_manager.cpp
    src/core/shader.cpp
    src/render/camera.cpp
    include/glad/glad.c
    src/render/chunk.cpp
)


add_executable(test ${SOURCE_FILES})


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(test PRIVATE glfw OpenGL::GL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ") #-Wall  -Wextra -Werror


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image)
include_directories(vector)

set_target_properties(test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
