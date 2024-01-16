cmake_minimum_required(VERSION 3.26)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(trees)

if (WIN32)
    # my windows configuration, if your libraries are in a different location, change this
    set(glfw3_DIR "/Program Files (x86)/GLFW/lib/cmake/glfw3")
    include_directories("/Program Files (x86)/glew/include")

else (WIN32)
    # linux and mac, assume glew can be found by cmake
    find_package(GLEW REQUIRED)
endif (WIN32)

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)


include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE all_srcs
    "${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/*.c"
)

add_compile_definitions(GLEW_NO_GLU GLEW_STATIC)

add_executable(trees ${all_srcs})

if (WIN32)
    target_link_libraries(trees glm::glm OpenGL::GL glfw "/Program Files (x86)/glew/lib/Release/Win32/glew32s.lib")
else (WIN32)
    target_link_libraries(trees glm::glm OpenGL::GL glfw GLEW::glew)
endif (WIN32)