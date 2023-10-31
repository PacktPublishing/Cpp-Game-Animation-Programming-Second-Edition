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

    ImGui::Text("Vertex Skinning:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Linear",
      renderData.rdGPUDualQuatVertexSkinning == skinningMode::linear)) {
       renderData.rdGPUDualQuatVertexSkinning = skinningMode::linear;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Dual Quaternion",
      renderData.rdGPUDualQuatVertexSkinning == skinningMode::dualQuat)) {
       renderData.rdGPUDualQuatVertexSkinning = skinningMode::dualQuat;
    }
  }

  if (ImGui::CollapsingHeader("glTF Animation")) {
    ImGui::Checkbox("Play Animation", &renderData.rdPlayAnimation);

    if (!renderData.rdPlayAnimation) {
      ImGui::BeginDisabled();
    }

    ImGui::Text("Animation Direction:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Forward",
      renderData.rdAnimationPlayDirection == replayDirection::forward)) {
       renderData.rdAnimationPlayDirection = replayDirection::forward;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Backward",
      renderData.rdAnimationPlayDirection == replayDirection::backward)) {
       renderData.rdAnimationPlayDirection = replayDirection::backward;
    }

    if (!renderData.rdPlayAnimation) {
      ImGui::EndDisabled();
    }

    ImGui::Text("Clip   ");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##ClipCombo",
      renderData.rdClipNames.at(renderData.rdAnimClip).c_str())) {
      for (int i = 0; i < renderData.rdClipNames.size(); ++i) {
        const bool isSelected = (renderData.rdAnimClip == i);
        if (ImGui::Selectable(renderData.rdClipNames.at(i).c_str(), isSelected)) {
          renderData.rdAnimClip = i;
        }

        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    if (renderData.rdPlayAnimation) {
      ImGui::Text("Speed  ");
      ImGui::SameLine();
      ImGui::SliderFloat("##ClipSpeed", &renderData.rdAnimSpeed, 0.0f, 2.0f, "%.3f", flags);
    } else {
      ImGui::Text("Timepos");
      ImGui::SameLine();
      ImGui::SliderFloat("##ClipPos", &renderData.rdAnimTimePosition, 0.0f,
        renderData.rdAnimEndTime, "%.3f", flags);
    }
  }

  if (ImGui::CollapsingHeader("glTF Animation Blending")) {
    ImGui::Text("Blending Type:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Fade In/Out",
      renderData.rdBlendingMode == blendMode::fadeinout)) {
       renderData.rdBlendingMode = blendMode::fadeinout;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Crossfading",
      renderData.rdBlendingMode == blendMode::crossfade)) {
       renderData.rdBlendingMode = blendMode::crossfade;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Additive",
      renderData.rdBlendingMode == blendMode::additive)) {
       renderData.rdBlendingMode = blendMode::additive;
    }

    if (renderData.rdBlendingMode == blendMode::fadeinout) {
      ImGui::Text("Blend Factor");
      ImGui::SameLine();
      ImGui::SliderFloat("##BlendFactor", &renderData.rdAnimBlendFactor, 0.0f, 1.0f, "%.3f",
        flags);
    }

    if (renderData.rdBlendingMode == blendMode::crossfade ||
        renderData.rdBlendingMode == blendMode::additive) {
      ImGui::Text("Dest Clip   ");
      ImGui::SameLine();
      if (ImGui::BeginCombo("##DestClipCombo",
        renderData.rdClipNames.at(renderData.rdCrossBlendDestAnimClip).c_str())) {
        for (int i = 0; i < renderData.rdClipNames.size(); ++i) {
          const bool isSelected = (renderData.rdCrossBlendDestAnimClip == i);
          if (ImGui::Selectable(renderData.rdClipNames.at(i).c_str(), isSelected)) {
            renderData.rdCrossBlendDestAnimClip = i;
          }

          if (isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }

      ImGui::Text("Cross Blend ");
      ImGui::SameLine();
      ImGui::SliderFloat("##CrossBlendFactor", &renderData.rdAnimCrossBlendFactor, 0.0f, 1.0f,
        "%.3f", flags);
    }

    if (renderData.rdBlendingMode == blendMode::additive) {
      ImGui::Text("Split Node  ");
      ImGui::SameLine();
      if (ImGui::BeginCombo("##SplitNodeCombo",
        renderData.rdSkelSplitNodeNames.at(renderData.rdSkelSplitNode).c_str())) {
        for (int i = 0; i < renderData.rdSkelSplitNodeNames.size(); ++i) {
          if (renderData.rdSkelSplitNodeNames.at(i).compare("(invalid)") != 0) {
            const bool isSelected = (renderData.rdSkelSplitNode == i);
            if (ImGui::Selectable(renderData.rdSkelSplitNodeNames.at(i).c_str(), isSelected)) {
              renderData.rdSkelSplitNode = i;
            }

            if (isSelected) {
              ImGui::SetItemDefaultFocus();
            }
          }
        }
        ImGui::EndCombo();
      }
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
