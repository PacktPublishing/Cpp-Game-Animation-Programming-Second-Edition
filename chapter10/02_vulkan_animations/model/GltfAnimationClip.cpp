#include "GltfAnimationClip.h"

GltfAnimationClip::GltfAnimationClip(std::string name) : mClipName(name)
{
}

void GltfAnimationClip::addChannel(std::shared_ptr<tinygltf::Model> model, tinygltf::Animation anim, tinygltf::AnimationChannel channel)
{
  std::shared_ptr<GltfAnimationChannel> chan = std::make_shared<GltfAnimationChannel>();
  chan->loadChannelData(model, anim, channel);
  mAnimationChannels.push_back(chan);
}

void GltfAnimationClip::setAnimationFrame(std::vector<std::shared_ptr<GltfNode>> nodes, float time)
{
  for (auto &channel : mAnimationChannels) {
    int targetNode = channel->getTargetNode();
    if (channel->getTargetPath() == ETargetPath::ROTATION) {
      nodes.at(targetNode)->setRotation(channel->getRotation(time));
    }
    if (channel->getTargetPath() == ETargetPath::TRANSLATION) {
      nodes.at(targetNode)->setTranslation(channel->getTranslation(time));
    }
    if (channel->getTargetPath() == ETargetPath::SCALE) {
      nodes.at(targetNode)->setScale(channel->getScaling(time));
    }
    nodes.at(targetNode)->calculateLocalTRSMatrix();
  }
}

float GltfAnimationClip::getClipEndTime() {
  return mAnimationChannels.at(0)->getMaxTime();;
}

std::string GltfAnimationClip::getClipName() {
  return mClipName;
}

