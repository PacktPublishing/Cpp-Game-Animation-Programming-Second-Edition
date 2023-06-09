/* glTF animation clip */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <tiny_gltf.h>

#include "GltfNode.h"
#include "GltfAnimationChannel.h"

class GltfAnimationClip {
  public:
    GltfAnimationClip(std::string name);
    void addChannel(std::shared_ptr<tinygltf::Model> model, tinygltf::Animation anim,
      tinygltf::AnimationChannel channel);
    void setAnimationFrame(std::vector<std::shared_ptr<GltfNode>> nodes, float time);
    void blendAnimationFrame(std::vector<std::shared_ptr<GltfNode>> nodes, float time,
      float blendFactor);
    float getClipEndTime();
    std::string getClipName();

  private:
    std::vector<std::shared_ptr<GltfAnimationChannel>> mAnimationChannels{};

    std::string mClipName;
};
