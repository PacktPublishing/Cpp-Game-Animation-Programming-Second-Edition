/* Vulkan shader */
#pragma once

#include <string>
#include <vulkan/vulkan.h>

class Shader {
  public:
    static VkShaderModule loadShader(VkDevice device, std::string shaderFileName);

  private:
    static std::string loadFileToString(std::string fileName);
};
