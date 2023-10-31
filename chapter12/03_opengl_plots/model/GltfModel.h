/* glTF model, ready to draw */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <glad/glad.h>
#include <tiny_gltf.h>

#include "Texture.h"
#include "GltfNode.h"
#include "GltfAnimationClip.h"

#include "OGLRenderData.h"

class GltfModel {
  public:
    bool loadModel(OGLRenderData &renderData, std::string modelFilename,
      std::string textureFilename);
    void draw();
    void cleanup();
    void uploadVertexBuffers();
    void uploadIndexBuffer();
    std::shared_ptr<OGLMesh> getSkeleton();
    int getJointMatrixSize();
    std::vector<glm::mat4> getJointMatrices();
    int getJointDualQuatsSize();
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

    void resetNodeData();
    void setSkeletonSplitNode(int nodeNum);
    std::string getNodeName(int nodeNum);

  private:
    void createVertexBuffers();
    void createIndexBuffer();
    int getTriangleCount();
    void getSkeletonPerNode(std::shared_ptr<GltfNode> treeNode);

    void getJointData();
    void getWeightData();
    void getInvBindMatrices();
    void getAnimations();
    void getNodes(std::shared_ptr<GltfNode> treeNode);
    void getNodeData(std::shared_ptr<GltfNode> treeNode, glm::mat4 parentNodeMatrix);

    void resetNodeData(std::shared_ptr<GltfNode> treeNode, glm::mat4 parentNodeMatrix);
    void updateNodeMatrices(std::shared_ptr<GltfNode> treeNode, glm::mat4 parentNodeMatrix);
    void updateJointMatricesAndQuats(std::shared_ptr<GltfNode> treeNode);
    void updateAdditiveMask(std::shared_ptr<GltfNode> treeNode, int splitNodeNum);

    std::vector<glm::tvec4<uint16_t>> mJointVec{};
    std::vector<glm::vec4> mWeightVec{};
    std::vector<glm::mat4> mInverseBindMatrices{};
    std::vector<glm::mat4> mJointMatrices{};
    std::vector<glm::mat2x4> mJointDualQuats{};

    std::vector<int> mAttribAccessors{};
    std::vector<int> mNodeToJoint{};


    std::shared_ptr<GltfNode> mRootNode = nullptr;
    std::shared_ptr<tinygltf::Model> mModel = nullptr;

    std::shared_ptr<OGLMesh> mSkeletonMesh = nullptr;

    std::vector<std::shared_ptr<GltfNode>> mNodeList;

    std::vector<std::shared_ptr<GltfAnimationClip>> mAnimClips{};

    std::vector<bool> mAdditiveAnimationMask{};
    std::vector<bool> mInvertedAdditiveAnimationMask{};

    GLuint mVAO = 0;
    std::vector<GLuint> mVertexVBO{};
    GLuint mIndexVBO = 0;
    std::map<std::string, GLint> attributes =
      {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}, {"JOINTS_0", 3}, {"WEIGHTS_0", 4}};

    Texture mTex{};
};
