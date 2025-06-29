cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use custom file to find libraries
if(WIN32)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
endif()

project(Main)

# optimize for speed on Clang or GCC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "-O3")
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Skip downloading if Vulkan SDK has been found
if (WIN32 AND NOT DEFINED ENV{VK_SDK_PATH})
  # get GLM
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        1.0.1
    GIT_SHALLOW 1
  )

  # GLM is header only, but needs to be populated for the download
  FetchContent_MakeAvailable(glm)

  FetchContent_GetProperties(glm)
  if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
    add_subdirectory(${glm_SOURCE_DIR} EXCLUDE_FROM_ALL)
  endif()

  # set variable that will be used in find_package
  set(GLM_INCLUDE_DIR ${glm_SOURCE_DIR})
endif()

file(GLOB SOURCES
  Main.cpp
  src/glad.c
  window/*.cpp
  tools/*.cpp
  opengl/*.cpp
  model/*.cpp
  imgui/*.cpp
  tinygltf/*.cc
)
add_executable(Main ${SOURCES})

target_include_directories(Main PUBLIC include src window tools opengl model imgui tinygltf)

find_package(glfw3 3.3 REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

# copy shader files
file(GLOB GLSL_SOURCE_FILES
  shader/*.frag
  shader/*.vert
)

add_custom_target(
  Shaders
  DEPENDS ${GLSL_SOURCE_FILES}
)
add_dependencies(Main Shaders)

add_custom_command(TARGET Shaders POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/shader"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/shader"
)

# copy textures
file(GLOB TEX_SOURCE_FILES
  textures/*
)

add_custom_target(
  Textures
  DEPENDS ${TEX_SOURCE_FILES}
)
add_dependencies(Main Textures)

add_custom_command(TARGET Textures POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/textures"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/textures"
)

# copy assets
file(GLOB ASSET_FILES
  assets/*
)

add_custom_target(
  Assets
  DEPENDS ${ASSET_FILES}
)

add_dependencies(Main Assets)

add_custom_command(TARGET Assets POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/assets"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/assets"
)

# variable is set by FindGLFW3.cmake, reuse for Linux
if(UNIX)
  set(GLFW3_LIBRARY glfw)
endif()

include_directories(${GLFW3_INCLUDE_DIR} ${GLM_INCLUDE_DIRS})

if(MSVC)
  target_link_libraries(Main ${GLFW3_LIBRARY} OpenGL::GL)
else()
  # Clang and GCC may need libstd++ and libmath
  target_link_libraries(Main ${GLFW3_LIBRARY} OpenGL::GL stdc++ m)
endif()
