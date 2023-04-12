#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "UserInterface.h"

void UserInterface::init(OGLRenderData &renderData) {
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGui_ImplGlfw_InitForOpenGL(renderData.rdWindow, true);

  const char *glslVersion = "#version 460 core";
  ImGui_ImplOpenGL3_Init(glslVersion);

  ImGui::StyleColorsDark();
}

void UserInterface::createFrame(OGLRenderData &renderData) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiWindowFlags imguiWindowFlags = 0;
  imguiWindowFlags |= ImGuiWindowFlags_NoCollapse;
  //imguiWindowFlags |= ImGuiWindowFlags_NoResize;
  //imguiWindowFlags |= ImGuiWindowFlags_NoMove;

  ImGui::SetNextWindowBgAlpha(0.8f);

  ImGui::Begin("Control", nullptr, imguiWindowFlags);

  static float newFps = 0.0f;
  /* avoid inf values (division by zero) */
  if (renderData.rdFrameTime > 0.0) {
    newFps = 1.0f / renderData.rdFrameTime * 1000.f;
  }
  /* make an averge value to avoid jumps */
  mFramesPerSecond = (mAveragingAlpha * mFramesPerSecond) + (1.0f - mAveragingAlpha) * newFps;

  ImGui::Text("FPS:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());

  ImGui::Separator();

  ImGui::Text("Frame Time:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdFrameTime).c_str());
  ImGui::SameLine();
  ImGui::Text("ms");

  ImGui::Text("Matrix Generation Time:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdMatrixGenerateTime).c_str());
  ImGui::SameLine();
  ImGui::Text("ms");

  ImGui::Text("Matrix Upload Time:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdUploadToUBOTime).c_str());
  ImGui::SameLine();
  ImGui::Text("ms");

  ImGui::Text("UI Generation Time:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdUIGenerateTime).c_str());
  ImGui::SameLine();
  ImGui::Text("ms");

  ImGui::Text("UI Draw Time:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdUIDrawTime).c_str());
  ImGui::SameLine();
  ImGui::Text("ms");

  ImGui::Separator();

  ImGui::Text("Camera Position:");
  ImGui::SameLine();
  ImGui::Text("%s", glm::to_string(renderData.rdCameraWorldPosition).c_str());

  ImGui::Text("View Azimuth:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdViewAzimuth).c_str());

  ImGui::Text("View Elevation:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdViewElevation).c_str());

  ImGui::Separator();

  ImGui::Text("Triangles:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(renderData.rdTriangleCount).c_str());

  std::string windowDims = std::to_string(renderData.rdWidth) + "x" + std::to_string(renderData.rdHeight);
  ImGui::Text("Window Dimensions:");
  ImGui::SameLine();
  ImGui::Text("%s", windowDims.c_str());

  std::string imgWindowPos = std::to_string(static_cast<int>(ImGui::GetWindowPos().x)) + "/" + std::to_string(static_cast<int>(ImGui::GetWindowPos().y));
  ImGui::Text("ImGui Window Position:");
  ImGui::SameLine();
  ImGui::Text("%s", imgWindowPos.c_str());

  ImGui::Separator();

  static bool checkBoxChecked = false;
  ImGui::Checkbox("Check Me", &checkBoxChecked);

  if (checkBoxChecked) {
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
    ImGui::Text("Yes");
    ImGui::PopStyleColor();
  }

  if (ImGui::Button("Toggle Shader")) {
    renderData.rdUseChangedShader = !renderData.rdUseChangedShader;
  }
  ImGui::SameLine();
  if (!renderData.rdUseChangedShader) {
    ImGui::Text("Basic Shader");
  } else {
    ImGui::Text("Changed Shader");
  }

  ImGui::Separator();

  ImGui::Text("Field of View");
  ImGui::SameLine();
  ImGui::SliderInt("##FOV", &renderData.rdFieldOfView, 40, 150);

  ImGui::End();
}

void UserInterface::render() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UserInterface::cleanup() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
