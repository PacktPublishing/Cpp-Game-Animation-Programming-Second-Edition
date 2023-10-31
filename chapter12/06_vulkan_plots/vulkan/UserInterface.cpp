#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "UserInterface.h"
#include "CommandBuffer.h"
#include "Logger.h"

bool UserInterface::init(VkRenderData& renderData) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  VkDescriptorPoolSize imguiPoolSizes[] =
  {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
  };

  VkDescriptorPoolCreateInfo imguiPoolInfo{};
  imguiPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  imguiPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  imguiPoolInfo.maxSets = 1000;
  imguiPoolInfo.poolSizeCount = std::size(imguiPoolSizes);
  imguiPoolInfo.pPoolSizes = imguiPoolSizes;

  if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &imguiPoolInfo, nullptr, &renderData.rdImguiDescriptorPool)) {
    Logger::log(1, "%s error: could not init ImGui descriptor pool \n", __FUNCTION__);
    return false;
  }

  ImGui_ImplGlfw_InitForVulkan(renderData.rdWindow, true);

  ImGui_ImplVulkan_InitInfo imguiIinitInfo{};
  imguiIinitInfo.Instance = renderData.rdVkbInstance.instance;
  imguiIinitInfo.PhysicalDevice = renderData.rdVkbPhysicalDevice.physical_device;
  imguiIinitInfo.Device = renderData.rdVkbDevice.device;
  imguiIinitInfo.Queue = renderData.rdGraphicsQueue;
  imguiIinitInfo.DescriptorPool = renderData.rdImguiDescriptorPool;
  imguiIinitInfo.MinImageCount = 2;
  imguiIinitInfo.ImageCount = renderData.rdSwapchainImages.size();
  imguiIinitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  ImGui_ImplVulkan_Init(&imguiIinitInfo, renderData.rdRenderpass);

  VkCommandBuffer imguiCommandBuffer;

  if (!CommandBuffer::init(renderData, imguiCommandBuffer)) {
    Logger::log(1, "%s error: could not create texture upload command buffers\n", __FUNCTION__);
    return false;
  }

  if (vkResetCommandBuffer(imguiCommandBuffer, 0) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to reset imgui command buffer\n", __FUNCTION__);
    return false;
  }

  VkCommandBufferBeginInfo cmdBeginInfo{};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if(vkBeginCommandBuffer(imguiCommandBuffer, &cmdBeginInfo) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to begin imgui command buffer\n", __FUNCTION__);
    return false;
  }

  ImGui_ImplVulkan_CreateFontsTexture(imguiCommandBuffer);

  if (vkEndCommandBuffer(imguiCommandBuffer) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to end staging command buffer\n", __FUNCTION__);
    return false;
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pWaitDstStageMask = nullptr;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = nullptr;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = nullptr;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &imguiCommandBuffer;;

  VkFence imguiBufferFence;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateFence(renderData.rdVkbDevice.device, &fenceInfo, nullptr, &imguiBufferFence) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to imgui buffer fence\n", __FUNCTION__);
    return false;
  }

  if (vkResetFences(renderData.rdVkbDevice.device, 1, &imguiBufferFence) != VK_SUCCESS) {
    Logger::log(1, "%s error: imgui buffer fence reset failed", __FUNCTION__);
    return false;
  }

  if (vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, imguiBufferFence) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to imgui init command buffer\n", __FUNCTION__);
    return false;
  }

  if (vkWaitForFences(renderData.rdVkbDevice.device, 1, &imguiBufferFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
    Logger::log(1, "%s error: waiting for imgui init fence failed", __FUNCTION__);
    return false;
  }

  vkDestroyFence(renderData.rdVkbDevice.device, imguiBufferFence, nullptr);
  CommandBuffer::cleanup(renderData, imguiCommandBuffer);

  ImGui_ImplVulkan_DestroyFontUploadObjects();

  ImGui::StyleColorsDark();

  /* init plot vectors */
  mFPSValues.resize(mNumFPSValues);
  mFrameTimeValues.resize(mNumFrameTimeValues);
  mModelUploadValues.resize(mNumModelUploadValues);
  mMatrixGenerationValues.resize(mNumMatrixGenerationValues);
  mMatrixUploadValues.resize(mNumMatrixUploadValues);
  mUiGenValues.resize(mNumUiGenValues);
  mUiDrawValues.resize(mNumUiDrawValues);

  return true;
}

void UserInterface::createFrame(VkRenderData& renderData) {
  ImGui_ImplVulkan_NewFrame();
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

  static double updateTime = 0.0;

  /* avoid double compares */
  if (updateTime < 0.000001) {
    updateTime = ImGui::GetTime();
  }

  static int fpsOffset = 0;
  static int frameTimeOffset = 0;
  static int modelUploadOffset = 0;
  static int matrixGenOffset = 0;
  static int matrixUploadOffset = 0;
  static int uiGenOffset = 0;
  static int uiDrawOffset = 0;

  if (updateTime < ImGui::GetTime()) {
    mFPSValues.at(fpsOffset) = mFramesPerSecond;
    fpsOffset = ++fpsOffset % mNumFPSValues;

    mFrameTimeValues.at(frameTimeOffset) = renderData.rdFrameTime;
    frameTimeOffset = ++frameTimeOffset % mNumFrameTimeValues;

    mModelUploadValues.at(modelUploadOffset) = renderData.rdUploadToVBOTime;
    modelUploadOffset = ++modelUploadOffset % mNumModelUploadValues;

    mMatrixGenerationValues.at(matrixGenOffset) = renderData.rdMatrixGenerateTime;
    matrixGenOffset = ++matrixGenOffset % mNumMatrixGenerationValues;

    mMatrixUploadValues.at(matrixUploadOffset) = renderData.rdUploadToUBOTime;
    matrixUploadOffset = ++matrixUploadOffset % mNumMatrixUploadValues;

    mUiGenValues.at(uiGenOffset) = renderData.rdUIGenerateTime;
    uiGenOffset = ++uiGenOffset % mNumUiGenValues;

    mUiDrawValues.at(uiDrawOffset) = renderData.rdUIDrawTime;
    uiDrawOffset = ++uiDrawOffset % mNumUiDrawValues;

    updateTime += 1.0 / 30.0;
  }

  ImGui::BeginGroup();
  ImGui::Text("FPS:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());
  ImGui::EndGroup();

  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    float averageFPS = 0.0f;
    for (const auto value : mFPSValues) {
      averageFPS += value;
    }
    averageFPS /= static_cast<float>(mNumFPSValues);
    std::string fpsOverlay = "now:     " + std::to_string(mFramesPerSecond) + "\n30s avg: " + std::to_string(averageFPS);
    ImGui::Text("FPS");
    ImGui::SameLine();
    ImGui::PlotLines("##FrameTimes", mFPSValues.data(), mFPSValues.size(), fpsOffset, fpsOverlay.c_str(), 0.0f, FLT_MAX,
      ImVec2(0, 80));
    ImGui::EndTooltip();
  }

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
    ImGui::BeginGroup();
    ImGui::Text("Frame Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdFrameTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      float averageFrameTime = 0.0f;
      for (const auto value : mFrameTimeValues) {
        averageFrameTime += value;
      }
      averageFrameTime /= static_cast<float>(mNumMatrixGenerationValues);
      std::string frameTimeOverlay = "now:     " + std::to_string(renderData.rdFrameTime)
        + " ms\n30s avg: " + std::to_string(averageFrameTime) + " ms";
      ImGui::Text("Frame Time       ");
      ImGui::SameLine();
      ImGui::PlotLines("##FrameTime", mFrameTimeValues.data(), mFrameTimeValues.size(), frameTimeOffset,
        frameTimeOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
      ImGui::EndTooltip();
    }

    ImGui::BeginGroup();
    ImGui::Text("Model Upload Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdUploadToVBOTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      float averageModelUpload = 0.0f;
      for (const auto value : mModelUploadValues) {
        averageModelUpload += value;
      }
      averageModelUpload /= static_cast<float>(mNumModelUploadValues);
      std::string modelUploadOverlay = "now:     " + std::to_string(renderData.rdUploadToVBOTime)
        + " ms\n30s avg: " + std::to_string(averageModelUpload) + " ms";
      ImGui::Text("VBO Upload");
      ImGui::SameLine();
      ImGui::PlotLines("##ModelUploadTimes", mModelUploadValues.data(), mModelUploadValues.size(), modelUploadOffset,
        modelUploadOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
      ImGui::EndTooltip();
    }

    ImGui::BeginGroup();
    ImGui::Text("Matrix Generation Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdMatrixGenerateTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      float averageMatGen = 0.0f;
      for (const auto value : mMatrixGenerationValues) {
        averageMatGen += value;
      }
      averageMatGen /= static_cast<float>(mNumMatrixGenerationValues);
      std::string matrixGenOverlay = "now:     " + std::to_string(renderData.rdMatrixGenerateTime)
        + " ms\n30s avg: " + std::to_string(averageMatGen) + " ms";
      ImGui::Text("Matrix Generation");
      ImGui::SameLine();
      ImGui::PlotLines("##MatrixGenTimes", mMatrixGenerationValues.data(), mMatrixGenerationValues.size(), matrixGenOffset,
        matrixGenOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
      ImGui::EndTooltip();
    }

    ImGui::BeginGroup();
    ImGui::Text("Matrix Upload Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdUploadToUBOTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      float averageMatrixUpload = 0.0f;
      for (const auto value : mMatrixUploadValues) {
        averageMatrixUpload += value;
      }
      averageMatrixUpload /= static_cast<float>(mNumMatrixUploadValues);
      std::string matrixUploadOverlay = "now:     " + std::to_string(renderData.rdUploadToVBOTime)
        + " ms\n30s avg: " + std::to_string(averageMatrixUpload) + " ms";
      ImGui::Text("UBO Upload");
      ImGui::SameLine();
      ImGui::PlotLines("##MatrixUploadTimes", mMatrixUploadValues.data(), mMatrixUploadValues.size(), matrixUploadOffset,
        matrixUploadOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
      ImGui::EndTooltip();
    }

    ImGui::BeginGroup();
    ImGui::Text("UI Generation Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdUIGenerateTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      float averageUiGen = 0.0f;
      for (const auto value : mUiGenValues) {
        averageUiGen += value;
      }
      averageUiGen /= static_cast<float>(mNumUiGenValues);
      std::string uiGenOverlay = "now:     " + std::to_string(renderData.rdUIGenerateTime)
        + " ms\n30s avg: " + std::to_string(averageUiGen) + " ms";
      ImGui::Text("UI Generation");
      ImGui::SameLine();
      ImGui::PlotLines("##ModelUpload", mUiGenValues.data(), mUiGenValues.size(), uiGenOffset,
        uiGenOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
      ImGui::EndTooltip();
    }

    ImGui::BeginGroup();
    ImGui::Text("UI Draw Time:");
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(renderData.rdUIDrawTime).c_str());
    ImGui::SameLine();
    ImGui::Text("ms");
    ImGui::EndGroup();

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      float averageUiDraw = 0.0f;
      for (const auto value : mUiDrawValues) {
        averageUiDraw += value;
      }
      averageUiDraw /= static_cast<float>(mNumUiDrawValues);
      std::string uiDrawOverlay = "now:     " + std::to_string(renderData.rdUIDrawTime)
        + " ms\n30s avg: " + std::to_string(averageUiDraw) + " ms";
      ImGui::Text("UI Draw");
      ImGui::SameLine();
      ImGui::PlotLines("##UIDrawTimes", mUiDrawValues.data(), mUiDrawValues.size(), uiGenOffset,
        uiDrawOverlay.c_str(), 0.0f, FLT_MAX, ImVec2(0, 80));
      ImGui::EndTooltip();
    }
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

void UserInterface::render(VkRenderData& renderData) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderData.rdCommandBuffer);
}

void UserInterface::cleanup(VkRenderData& renderData) {
  vkDestroyDescriptorPool(renderData.rdVkbDevice.device, renderData.rdImguiDescriptorPool, nullptr);
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}
