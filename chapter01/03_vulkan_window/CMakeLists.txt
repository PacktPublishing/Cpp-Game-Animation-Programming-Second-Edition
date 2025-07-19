cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use custom file to find libraries
if(WIN32)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
endif()

project(Main)

file(GLOB SOURCES
  Main.cpp
  window/*.cpp
  tools/*.cpp
)

add_executable(Main ${SOURCES})

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

# variable is set by FindGLFW3.cmake, reuse for Linux
if(UNIX)
  set(GLFW3_LIBRARY glfw)
endif()

include_directories(${GLFW3_INCLUDE_DIR} window tools)

# enable experimental GLM functions globally
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

if(MSVC)
  target_link_libraries(Main ${GLFW3_LIBRARY} Vulkan::Vulkan)
else()
  # Clang and GCC may need libstd++
  target_link_libraries(Main ${GLFW3_LIBRARY} Vulkan::Vulkan stdc++)
endif()
