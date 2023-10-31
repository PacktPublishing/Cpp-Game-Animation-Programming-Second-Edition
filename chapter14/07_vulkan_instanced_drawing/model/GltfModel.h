/* glTF model, ready to draw */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <vulkan/vulkan.h>
#include <tiny_gltf.h>

#include "Texture.h"
#include "GltfNode.h"
#include "GltfAnimationClip.h"

#include "VkRenderData.h"
#include "ModelSettings.h"

struct GltfNodeData {
    std::shared_ptr<GltfNode> rootNode;
    std::vector<std::shared_ptr<GltfNode>> nodeList;
};

class GltfModel {
  public:
    bool loadModel(VkRenderData &renderData, std::string modelFilename,
      std::string textureFilename);
    void draw(VkRenderData &renderData);
    void drawInstanced(VkRenderData &renderData, int instanceCount);
    void cleanup(VkRenderData &renderData);
    void uploadVertexBuffers(VkRenderData& renderData);
    void uploadIndexBuffer(VkRenderData& renderData);
    VkTextureData getVkTextureData();

    std::string getModelFilename();
    int getNodeCount();
    GltfNodeData getGltfNodes();
    int getTriangleCount();

    std::vector<glm::mat4> getInverseBindMatrices();
    std::vector<int> getNodeToJoint();

    std::vector<std::shared_ptr<GltfAnimationClip>> getAnimClips();

    void resetNodeData(std::shared_ptr<GltfNode> treeNode);

  private:
    void createVertexBuffers(VkRenderData& renderData);
    void createIndexBuffer(VkRenderData& renderData);

    void getJointData();
    void getWeightData();
    void getInvBindMatrices();
    void getAnimations();
    void getNodes(std::shared_ptr<GltfNode> treeNode);
    void getNodeData(std::shared_ptr<GltfNode> treeNode);
    std::vector<std::shared_ptr<GltfNode>> getNodeList(std::vector<std::shared_ptr<GltfNode>>
      &nodeList, int nodeNum);

    int mNodeCount = 0;
    std::string mModelFilename;

    std::shared_ptr<tinygltf::Model> mModel = nullptr;

    std::vector<glm::tvec4<uint16_t>> mJointVec{};
    std::vector<glm::vec4> mWeightVec{};
    std::vector<glm::mat4> mInverseBindMatrices{};

    std::vector<int> mAttribAccessors{};
    std::vector<int> mNodeToJoint{};

    std::vector<std::shared_ptr<GltfAnimationClip>> mAnimClips{};

    VkGltfRenderData mGltfRenderData{};

    std::map<std::string, GLint> attributes =
      {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}, {"JOINTS_0", 3}, {"WEIGHTS_0", 4}};
};
