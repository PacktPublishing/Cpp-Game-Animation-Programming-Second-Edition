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
  imguiIinitInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiIinitInfo.PipelineInfoMain.RenderPass = renderData.rdRenderpass;

  if (!ImGui_ImplVulkan_Init(&imguiIinitInfo)) {
    Logger::log(1, "%s error: could not init ImGui for Vulkan \n", __FUNCTION__);
    return false;
  }

  ImGui::StyleColorsDark();

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

  ImGui::Text("FPS:");
  ImGui::SameLine();
  ImGui::Text("%s", std::to_string(mFramesPerSecond).c_str());

  if (ImGui::CollapsingHeader("Info")) {
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
    ImGui::SliderInt("##FOV", &renderData.rdFieldOfView, 40, 150);
  }

  if (ImGui::CollapsingHeader("Angles")) {
    ImGui::Checkbox("Draw World Coordinate Arrows", &renderData.rdDrawWorldCoordArrows);
    ImGui::Checkbox("Draw Model Coordinate Arrows", &renderData.rdDrawModelCoordArrows);

    if (ImGui::Button("Reset Rotation")) {
      renderData.rdResetAngles = true;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
    ImGui::Text("X Rotation");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SliderInt("##ROTX", &renderData.rdRotXAngle, 0, 360);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
    ImGui::Text("Y Rotation");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SliderInt("##ROTY", &renderData.rdRotYAngle, 0, 360);

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
    ImGui::Text("Z Rotation");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::SliderInt("##ROTZ", &renderData.rdRotZAngle, 0, 360);
  }

  ImGui::End();
}

void UserInterface::render(VkRenderData& renderData) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderData.rdCommandBuffer);
}

void UserInterface::cleanup(VkRenderData& renderData) {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  vkDestroyDescriptorPool(renderData.rdVkbDevice.device, renderData.rdImguiDescriptorPool, nullptr);
  ImGui::DestroyContext();
}
