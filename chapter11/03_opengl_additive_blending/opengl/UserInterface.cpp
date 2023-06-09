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
  //imguiWindowFlags |= ImGuiWindowFlags_NoCollapse;
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

  /* clamp manual input on all sliders to min/max */
  ImGuiSliderFlags flags = ImGuiSliderFlags_ClampOnInput;

  ImGui::Text("FPS:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());

  if (ImGui::CollapsingHeader("Info")) {
    ImGui::Text("Triangles:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdTriangleCount + renderData.rdGltfTriangleCount).c_str());

    std::string windowDims = std::to_string(renderData.rdWidth) + "x" + std::to_string(renderData.rdHeight);
    ImGui::Text("Window Dimensions:");
    ImGui::SameLine();
    ImGui::Text("%s", windowDims.c_str());

    std::string imgWindowPos = std::to_string(static_cast<int>(ImGui::GetWindowPos().x)) + "/" + std::to_string(static_cast<int>(ImGui::GetWindowPos().y));
    ImGui::Text("ImGui Window Position:");
    ImGui::SameLine();
    ImGui::Text("%s", imgWindowPos.c_str());
  }

  if (ImGui::CollapsingHeader("Timers")) {
    ImGui::Text("Frame Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdFrameTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");

    ImGui::Text("Model Upload Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdUploadToVBOTime).c_str());
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
  }

  if (ImGui::CollapsingHeader("Camera")) {
    ImGui::Text("Camera Position:");
    ImGui::SameLine();
    ImGui::Text("%s", glm::to_string(renderData.rdCameraWorldPosition).c_str());

    ImGui::Text("View Azimuth:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdViewAzimuth).c_str());

    ImGui::Text("View Elevation:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdViewElevation).c_str());

    ImGui::Text("Field of View");
    ImGui::SameLine();
    ImGui::SliderInt("##FOV", &renderData.rdFieldOfView, 40, 150, "%d", flags);
  }

  if (ImGui::CollapsingHeader("glTF Model")) {
    ImGui::Checkbox("Draw Model", &renderData.rdDrawGltfModel);
    ImGui::Checkbox("Draw Skeleton", &renderData.rdDrawSkeleton);

    ImGui::Checkbox("GPU Vertex Skinning Method:", &renderData.rdGPUDualQuatVertexSkinning);
    ImGui::SameLine();
    if (renderData.rdGPUDualQuatVertexSkinning) {
      ImGui::Text("Dual Quaternion");
    } else {
      ImGui::Text("Linear");
    }
  }

  if (ImGui::CollapsingHeader("glTF Animation")) {
    ImGui::Text("Clip No");
    ImGui::SameLine();
    ImGui::SliderInt("##Clip", &renderData.rdAnimClip, 0, renderData.rdAnimClipSize - 1,
      "%d", flags);

    ImGui::Text("Clip Name: %s", renderData.rdClipName.c_str());

    ImGui::Checkbox("Play Animation", &renderData.rdPlayAnimation);

    ImGui::Checkbox("Animation Direction:", &renderData.rdPlayAnimationBackward);
    ImGui::SameLine();
    if (!renderData.rdPlayAnimationBackward) {
      ImGui::Text("Forward");
    } else {
      ImGui::Text("Backward");
    }

    if (!renderData.rdPlayAnimation) {
      ImGui::BeginDisabled();
    }
    ImGui::Text("Speed  ");
    ImGui::SameLine();
    ImGui::SliderFloat("##ClipSpeed", &renderData.rdAnimSpeed, 0.0f, 2.0f, "%.3f", flags);
    if (!renderData.rdPlayAnimation) {
      ImGui::EndDisabled();
    }

    if (renderData.rdPlayAnimation) {
      ImGui::BeginDisabled();
    }
    ImGui::Text("Timepos");
    ImGui::SameLine();
    ImGui::SliderFloat("##ClipPos", &renderData.rdAnimTimePosition, 0.0f,
      renderData.rdAnimEndTime, "%.3f", flags);

    if (renderData.rdPlayAnimation) {
      ImGui::EndDisabled();
    }
  }

  if (ImGui::CollapsingHeader("glTF Animation Blending")) {
    ImGui::Checkbox("Blending Type:", &renderData.rdCrossBlending);
    ImGui::SameLine();
    if (renderData.rdCrossBlending) {
      ImGui::Text("Cross");
    } else {
      ImGui::Text("Single");
    }

    if (renderData.rdCrossBlending) {
      ImGui::BeginDisabled();
    }

    ImGui::Text("Blend Factor");
    ImGui::SameLine();
    ImGui::SliderFloat("##BlendFactor", &renderData.rdAnimBlendFactor, 0.0f, 1.0f, "%.3f",
      flags);

    if (renderData.rdCrossBlending) {
      ImGui::EndDisabled();
    }

    if (!renderData.rdCrossBlending) {
      ImGui::BeginDisabled();
    }

    ImGui::Text("Dest Clip   ");
    ImGui::SameLine();
    ImGui::SliderInt("##DestClip", &renderData.rdCrossBlendDestAnimClip, 0,
      renderData.rdAnimClipSize - 1, "%d", flags);

    ImGui::Text("Dest Clip Name: %s", renderData.rdCrossBlendDestClipName.c_str());

    ImGui::Text("Cross Blend ");
    ImGui::SameLine();
    ImGui::SliderFloat("##CroosBlendFactor", &renderData.rdAnimCrossBlendFactor, 0.0f, 1.0f,
      "%.3f", flags);

    ImGui::Checkbox("Additive Blending", &renderData.rdAdditiveBlending);

    if (!renderData.rdAdditiveBlending) {
      ImGui::BeginDisabled();
    }
    ImGui::Text("Split Node  ");
    ImGui::SameLine();
    ImGui::SliderInt("##SplitNode", &renderData.rdSkelSplitNode, 0,
      renderData.rdModelNodeCount - 1, "%d", flags);
    ImGui::Text("Split Node Name: %s", renderData.rdSkelSplitNodeName.c_str());

    if (!renderData.rdAdditiveBlending) {
      ImGui::EndDisabled();
    }
    if (!renderData.rdCrossBlending) {
      ImGui::EndDisabled();
    }

  }

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
