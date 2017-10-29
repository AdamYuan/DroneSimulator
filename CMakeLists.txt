cmake_minimum_required(VERSION 3.8)
project(ProblemA)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIRS})
    link_libraries(${OPENGL_LIBRARIES})
endif()

find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules" ${CMAKE_MODULE_PATH})

find_package(GLFW3 REQUIRED)
if (GLFW3_FOUND)
    include_directories(${GLFW3_INCLUDE_DIR})
    link_libraries(${GLFW3_LIBRARY})
endif()

find_package(SOIL REQUIRED)
if (SOIL_FOUND)
    include_directories(${SOIL_INCLUDE_DIRS})
    link_libraries(${SOIL_LIBRARIES})
endif()

include_directories(dep)
file(GLOB SOURCE_FILES src/*.cpp dep/MyGL/*.cpp)

add_executable(ProblemA ${SOURCE_FILES})