/* glTF model instance */
#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "GltfModel.h"
#include "GltfNode.h"
#include "GltfAnimationClip.h"

#include "VkRenderData.h"
#include "ModelSettings.h"

class GltfInstance {
  public:
    GltfInstance(std::shared_ptr<GltfModel> model, glm::vec2 worldPos, bool randomize = false);
    ~GltfInstance();

    void resetNodeData();

    std::shared_ptr<VkMesh> getSkeleton();
    void setSkeletonSplitNode(int nodeNum);

    int getJointMatrixSize();
    int getJointDualQuatsSize();
    std::vector<glm::mat4> getJointMatrices();
    std::vector<glm::mat2x4> getJointDualQuats();

    void playAnimation(int animNum, float speedDivider, float blendFactor,
      replayDirection direction);
    void playAnimation(int sourceAnimNum, int destAnimNum, float speedDivider,
      float blendFactor, replayDirection direction);

    void blendAnimationFrame(int animNumber, float time, float blendFactor);
    void crossBlendAnimationFrame(int sourceAnimNumber, int destAnimNumber, float time,
      float blendFactor);

    float getAnimationEndTime(int animNum);
    std::string getClipName(int animNum);

    void setInstanceSettings(ModelSettings settings);
    ModelSettings getInstanceSettings();
    void checkForUpdates();

    glm::vec2 getWorldPosition();
    glm::quat getWorldRotation();

  private:
    void getSkeletonPerNode(std::shared_ptr<GltfNode> treeNode);
    void updateNodeMatrices(std::shared_ptr<GltfNode> treeNode);
    void updateJointMatrices(std::shared_ptr<GltfNode> treeNode);
    void updateJointDualQuats(std::shared_ptr<GltfNode> treeNode);
    void updateAdditiveMask(std::shared_ptr<GltfNode> treeNode, int splitNodeNum);

    std::shared_ptr<GltfModel> mGltfModel = nullptr;
    unsigned int mNodeCount = 0;

    /* every model needs its onw set of nodes */
    std::shared_ptr<GltfNode> mRootNode = nullptr;
    std::vector<std::shared_ptr<GltfNode>> mNodeList{};

    std::vector<std::shared_ptr<GltfAnimationClip>> mAnimClips{};
    std::vector<glm::mat4> mInverseBindMatrices{};
    std::vector<glm::mat4> mJointMatrices{};
    std::vector<glm::mat2x4> mJointDualQuats{};

    std::vector<int> mNodeToJoint{};

    std::vector<bool> mAdditiveAnimationMask{};
    std::vector<bool> mInvertedAdditiveAnimationMask{};

    std::shared_ptr<VkMesh> mSkeletonMesh = nullptr;

    ModelSettings mModelSettings{};
};
