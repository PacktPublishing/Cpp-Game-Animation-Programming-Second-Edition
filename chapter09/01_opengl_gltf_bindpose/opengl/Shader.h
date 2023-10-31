#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader {
  public:
    bool loadShaders(std::string vertexShaderFileName, std::string fragmentShaderFileName);
    void use();
    void cleanup();

  private:
    GLuint mShaderProgram = 0;

    bool createShaderProgram(std::string vertexShaderFileName, std::string fragmentShaderFileName);
    GLuint loadShader(std::string shaderFileName, GLuint shaderType);
    std::string loadFileToString(std::string filename);
    bool checkCompileStats(std::string shaderFileName, GLuint shader);
    bool checkLinkStats(std::string vertexShaderFileName, std::string fragmentShaderFileName, GLuint shaderProgram);
};
