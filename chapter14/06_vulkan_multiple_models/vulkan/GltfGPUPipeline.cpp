#include <vector>

#include "GltfGPUPipeline.h"
#include "Logger.h"
#include "Shader.h"

#include <glm/glm.hpp>
#include <VkBootstrap.h>

bool GltfGPUPipeline::init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout,
    VkPipeline& pipeline, VkPrimitiveTopology topology,
    std::string vertexShaderFilename, std::string fragmentShaderFilename) {
  /* shader */
  VkShaderModule vertexModule = Shader::loadShader(renderData.rdVkbDevice.device,
    vertexShaderFilename);
  VkShaderModule fragmentModule = Shader::loadShader(renderData.rdVkbDevice.device,
    fragmentShaderFilename);

  if (vertexModule == VK_NULL_HANDLE || fragmentModule == VK_NULL_HANDLE) {
    Logger::log(1, "%s error: could not load shaders\n", __FUNCTION__);
    return false;
  }

  VkPipelineShaderStageCreateInfo vertexStageInfo{};
  vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertexStageInfo.module = vertexModule;
  vertexStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragmentStageInfo{};
  fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragmentStageInfo.module = fragmentModule;
  fragmentStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStagesInfo[] = { vertexStageInfo, fragmentStageInfo };

  /* assemble the graphics pipeline itself */
  VkVertexInputBindingDescription vertexBindings[5];
  vertexBindings[0].binding = 0;
  vertexBindings[0].stride = sizeof(glm::vec3);
  vertexBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertexBindings[1].binding = 1;
  vertexBindings[1].stride = sizeof(glm::vec3);
  vertexBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertexBindings[2].binding = 2;
  vertexBindings[2].stride = sizeof(glm::vec2);
  vertexBindings[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertexBindings[3].binding = 3;
  vertexBindings[3].stride = sizeof(uint16_t) * 4; // 4x unsigned short
  vertexBindings[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertexBindings[4].binding = 4;
  vertexBindings[4].stride = sizeof(glm::vec4);
  vertexBindings[4].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription positionAttribute{};
  positionAttribute.binding = 0;
  positionAttribute.location = 0;
  positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
  positionAttribute.offset = 0;

  VkVertexInputAttributeDescription normalAttribute{};
  normalAttribute.binding = 1;
  normalAttribute.location = 1;
  normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
  normalAttribute.offset = 0;

  VkVertexInputAttributeDescription uvAttribute{};
  uvAttribute.binding = 2;
  uvAttribute.location = 2;
  uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
  uvAttribute.offset = 0;

  VkVertexInputAttributeDescription jointsAttribute{};
  jointsAttribute.binding = 3;
  jointsAttribute.location = 3;
  jointsAttribute.format = VK_FORMAT_R16G16B16A16_UINT; // 4x unsigned short
  jointsAttribute.offset = 0;

  VkVertexInputAttributeDescription weightAttribute{};
  weightAttribute.binding = 4;
  weightAttribute.location = 4;
  weightAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  weightAttribute.offset = 0;

  VkVertexInputAttributeDescription attributes[] =
    { positionAttribute, normalAttribute, uvAttribute, jointsAttribute, weightAttribute };

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 5;
  vertexInputInfo.pVertexBindingDescriptions = vertexBindings;
  vertexInputInfo.vertexAttributeDescriptionCount = 5;
  vertexInputInfo.pVertexAttributeDescriptions = attributes;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
  inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyInfo.topology = topology;
  inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(renderData.rdVkbSwapchain.extent.width);
  viewport.height = static_cast<float>(renderData.rdVkbSwapchain.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = renderData.rdVkbSwapchain.extent;

  VkPipelineViewportStateCreateInfo viewportStateInfo{};
  viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateInfo.viewportCount = 1;
  viewportStateInfo.pViewports = &viewport;
  viewportStateInfo.scissorCount = 1;
  viewportStateInfo.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
  rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizerInfo.depthClampEnable = VK_FALSE;
  rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizerInfo.lineWidth = 1.0f;
  rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  /* set to CCW to match the inverted viewport from OpenGL */
  rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizerInfo.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
  multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisamplingInfo.sampleShadingEnable = VK_FALSE;
  multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
  colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendingInfo.logicOpEnable = VK_FALSE;
  colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
  colorBlendingInfo.attachmentCount = 1;
  colorBlendingInfo.pAttachments = &colorBlendAttachment;
  colorBlendingInfo.blendConstants[0] = 0.0f;
  colorBlendingInfo.blendConstants[1] = 0.0f;
  colorBlendingInfo.blendConstants[2] = 0.0f;
  colorBlendingInfo.blendConstants[3] = 0.0f;

  VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
  depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilInfo.depthTestEnable = VK_TRUE;
  depthStencilInfo.depthWriteEnable = VK_TRUE;
  depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  depthStencilInfo.minDepthBounds = 0.0f;
  depthStencilInfo.maxDepthBounds = 1.0f;
  depthStencilInfo.stencilTestEnable = VK_FALSE;

  std::vector<VkDynamicState> dynStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };

  VkPipelineDynamicStateCreateInfo dynStatesInfo{};
  dynStatesInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynStatesInfo.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
  dynStatesInfo.pDynamicStates = dynStates.data();

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStagesInfo;
  pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
  pipelineCreateInfo.pViewportState = &viewportStateInfo;
  pipelineCreateInfo.pRasterizationState = &rasterizerInfo;
  pipelineCreateInfo.pMultisampleState = &multisamplingInfo;
  pipelineCreateInfo.pColorBlendState = &colorBlendingInfo;
  pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
  pipelineCreateInfo.pDynamicState = &dynStatesInfo;
  pipelineCreateInfo.layout = pipelineLayout;
  pipelineCreateInfo.renderPass = renderData.rdRenderpass;
  pipelineCreateInfo.subpass = 0;
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(renderData.rdVkbDevice.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create rendering pipeline\n", __FUNCTION__);
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
    return false;
  }

  /* it is save to destroy the shader modules after pipeline has been created */
  vkDestroyShaderModule (renderData.rdVkbDevice.device, fragmentModule, nullptr);
  vkDestroyShaderModule (renderData.rdVkbDevice.device, vertexModule, nullptr);

  return true;
}

void GltfGPUPipeline::cleanup(VkRenderData &renderData, VkPipeline &pipeline) {
  vkDestroyPipeline(renderData.rdVkbDevice.device, pipeline, nullptr);
}
