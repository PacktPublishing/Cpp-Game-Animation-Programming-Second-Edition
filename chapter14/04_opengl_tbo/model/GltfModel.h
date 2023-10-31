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

struct GltfNodeData {
    std::shared_ptr<GltfNode> rootNode;
    std::vector<std::shared_ptr<GltfNode>> nodeList;
};

class GltfModel {
  public:
    bool loadModel(OGLRenderData &renderData, std::string modelFilename,
      std::string textureFilename);
    void draw();
    void drawInstanced(int instanceCount);
    void cleanup();

    std::string getModelFilename();
    int getNodeCount();
    GltfNodeData getGltfNodes();
    int getTriangleCount();

    void uploadVertexBuffers();
    void uploadIndexBuffer();

    std::vector<glm::mat4> getInverseBindMatrices();
    std::vector<int> getNodeToJoint();

    std::vector<std::shared_ptr<GltfAnimationClip>> getAnimClips();

    void resetNodeData(std::shared_ptr<GltfNode> treeNode);

  private:
    void createVertexBuffers();
    void createIndexBuffer();

    void getJointData();
    void getWeightData();
    void getInvBindMatrices();
    void getAnimations();
    void getNodes(std::shared_ptr<GltfNode> treeNode);
    void getNodeData(std::shared_ptr<GltfNode> treeNode);
    std::vector<std::shared_ptr<GltfNode>> getNodeList(std::vector<std::shared_ptr<GltfNode>>
      &nodeList, int nodeNum);

    std::string mModelFilename;
    int mNodeCount = 0;

    std::shared_ptr<tinygltf::Model> mModel = nullptr;

    std::vector<glm::tvec4<uint16_t>> mJointVec{};
    std::vector<glm::vec4> mWeightVec{};
    std::vector<glm::mat4> mInverseBindMatrices{};

    std::vector<int> mAttribAccessors{};
    std::vector<int> mNodeToJoint{};

    std::vector<std::shared_ptr<GltfAnimationClip>> mAnimClips{};

    GLuint mVAO = 0;
    std::vector<GLuint> mVertexVBO{};
    GLuint mIndexVBO = 0;
    std::map<std::string, GLint> attributes =
      {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}, {"JOINTS_0", 3}, {"WEIGHTS_0", 4}};

    Texture mTex{};
};
