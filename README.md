# C++ Game Animation Programming

<a href="https://www.packtpub.com/product/c-game-animation-programming-second-edition/9781803246529"><img src="https://content.packt.com/B18196/cover_image_small.jpg" alt="" height="256px" align="right"></a>

This is the code repository for [C++ Game Animation Programming](https://www.packtpub.com/product/c-game-animation-programming-second-edition/9781803246529), published by Packt.

**Learn modern animation techniques from theory to implementation using C++, OpenGL, and Vulkan**

## What is this book about?
If you‘re fascinated by the complexities of animating video game characters and are curious about the transformation of model files into 3D avatars and NPCs that can explore virtual worlds, then this book is for you.

This book covers the following exciting features:
* Create simple OpenGL and Vulkan applications and work with shaders
* Explore the glTF file format, including its design and data structures
* Design an animation system with poses, clips, and skinned meshes
* Find out how vectors, matrices, quaternions, and splines are used in game development
* Discover and implement ways to seamlessly blend character animations
* Implement inverse kinematics for your characters using CCD and FABRIK solvers
* Understand how to render large, animated crowds efficiently
* Identify and resolve performance issues

If you feel this book is for you, get your [copy](https://www.amazon.com/dp/1803246529) today!

<a href="https://www.packtpub.com/?utm_source=github&utm_medium=banner&utm_campaign=GitHubBanner"><img src="https://raw.githubusercontent.com/PacktPublishing/GitHub/master/GitHub.png" 
alt="https://www.packtpub.com/" border="5" /></a>

## Instructions and Navigations
All of the code is organized into folders.


**Following is what you need for this book:**
This book is for curious C++ developers, game programmers, game designers, and character animators, either pursuing this as a hobby or profession, who have always wanted to look behind the curtain and see how character animation in games works. The book assumes basic C++ and math knowledge, and you should be able to read code and math formulas to get the most out of this book.

With the following software and hardware list you can run all code files present in the book (Chapter 1-15).

### Software and Hardware List
| Chapter | Software required | OS required |
| -------- | ------------------------------------ | ----------------------------------- |
| 1-15 | Visual Studio 2022 or Eclipse 2023-06 (or newer) | Windows 10 (or newer), or Linux (i.e. Ubuntu 22.04, Debian 12, or newer) |


### Related products
* Mathematics for Game Programming and Computer Graphics [[Packt]](https://www.packtpub.com/product/mathematics-for-game-programming-and-computer-graphics/9781801077330) [[Amazon]](https://www.amazon.com/dp/1801077339)

* Beginning C++ Game Programming [[Packt]](https://www.packtpub.com/product/beginning-c-game-programming-second-edition/9781838648572) [[Amazon]](https://www.amazon.com/dp/1838648577)


## Get to Know the Authors
**Michael Dunsky**
is an educated electronics technician, game developer, and console porting programmer with more than 20 years of programming experience. He started at the age of 14 with BASIC, adding on his way Assembly language, C, C++, Java, Python, VHDL, OpenGL, GLSL, and Vulkan to his portfolio. During his career, he also gained extensive knowledge in virtual machines, server operation, infrastructure automation, and other DevOps topics. Michael holds a Master of Science degree in Computer Science from the FernUniversität in Hagen, focused on computer graphics, parallel programming and software systems.

**Gabor Szauer**
has been making games since 2010. He graduated from Full Sail University in 2010 with a bachelor's degree in game development. Gabor maintains an active Twitter presence, and maintains a programming-oriented game development blog. Gabor's previously published books are Game Physics Programming Cookbook and Lua Quick Start Guide, both published by Packt.

## Author Notes & Recent Fixes
The following section contains updates within the text of the book.

### Chapter 2
CMake needs two additional custom targets to copy the shaders and textures to the correct path, relative to the executable file.

Add the following lines to the file `CMakeLists.txt` in the root folder of the `opengl_renderer` project, right after the `find_package` lines:
```
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
```



Another update is needed for the `Model` file, see page 71 of the book.

The line `mVertexData.vertices.resize(6)` must be added to the file `model/Model.cpp`, right after the start of the `init()` method:
```
void Model::init() {
  mVertexData.vertices.resize(6);

  mVertexData.vertices[0].position =  glm::vec3(-0.5f,
    -0.5f,  0.5f);
  ...
}
```

### Chapter 3
A similar change as for Chapter 2 is needed for Chapter three. For Vulkan, we also need to compile the shaders before storing the SPIR-V code next to the executable file.

Add the following lines to the file `CMakeLists.txt` in the root folder of the `vulkan_renderer` project, right after the `find_package` lines:
```
# compile shaders
file(GLOB GLSL_SOURCE_FILES
  shader/*.frag
  shader/*.vert
)

if(Vulkan_GLSLC_EXECUTABLE)
  message("Using glslc to compile shaders")
  foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_SOURCE_DIR}/shader/${FILE_NAME}.spv")
    add_custom_command(
      OUTPUT ${SPIRV}
      COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${SPIRV} ${GLSL}
      DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
  endforeach(GLSL)
elseif (Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
  message("Using glslangValidator to compile shaders")
  foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_SOURCE_DIR}/shader/${FILE_NAME}.spv")
    add_custom_command(
      OUTPUT ${SPIRV}
      COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V -o ${SPIRV} ${GLSL}
      DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
  endforeach(GLSL)
endif()

add_custom_target(
  Shaders
  DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(Main Shaders)

add_custom_command(TARGET Shaders POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/shader"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/shader"
)

# copy changed textures
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
```

### Chapter 4
During the initialization of the renderer in the `Window` class in Chapter 4, the GLFW window handle needs to be handed over to the renderer to be able to use the `glfwGetKey()` call. We also need to store width and height of the window

Add a declaration of the constructor in the file `opengl/OGLRenderer.h` to have a `GLFWwindow` as the only parameter, and add three new private member variables named `mWindow`, `mWidth`, and `mHeight`:
```
  public:
    OGLRenderer(GLFWwindow *window);
    ...

  private:
    GLFWwindow* mWindow = nullptr;
    int mWidth = 0;
    int mHeight = 0;
    ....
```

In the file `opengl/OGLRenderer.cpp`, add the constructor definition and store the window value in the `mWindow` variable:
```
OGLRenderer::OGLRenderer(GLFWwindow *window) {
  mWindow = window;
}
```

At the beginning of the `init()` call, store the width and height in the new variables `mWidth` and `mHeight`:
```
bool OGLRenderer::init(unsigned int width, unsigned int height) {
  mWidth = width;
  mHeight = height;
  ...
```

As last step, add the `mWindow` variable in the file `window/Window.cpp` as parameter to the creation of the renderer:
```
  mRenderer = std::make_unique<OGLRenderer>(mWindow);
```

There's also a correction to the text on page 123:
In the third paragraph, two functions are named, `init()` and `resize()`. The correct name of the second function is `setSize()`.

### Chapter 10
Autocorrection did some unwanted changes to the two enum class names `ETargetPath` and `EInterpolationType`: the second letter is written in lowercase instead of uppercase.

For `ETargetPath`, the following pages are affected: 284 (code snippet), 286 (two code snippets), 287 (code snippet).
For `EInterpolationType`, the follwing pages are affected: 283 (text), 286 (code snippet), 289 (three code snippets).

