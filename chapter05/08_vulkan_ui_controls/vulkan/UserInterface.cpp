#include <string>

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

  return true;
}

void UserInterface::createFrame(VkRenderData& renderData) {
  ImGui_ImplVulkan_NewFrame();
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
