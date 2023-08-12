#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "IKSolver.h"
#include "Logger.h"

IKSolver::IKSolver() : IKSolver(9) {}

IKSolver::IKSolver(unsigned int iterations) : mIterations(iterations) {}

void IKSolver::setNumIterations(unsigned int iterations) {
  mIterations = iterations;
}

void IKSolver::setNodes(std::vector<std::shared_ptr<GltfNode>> nodes) {
  mNodes = nodes;
  for (const auto &node : mNodes) {
    if (node) {
      Logger::log(2, "%s: added node %s to IK solver\n", __FUNCTION__,
        node->getNodeName().c_str());
    }
  }
  calculateBoneLengths();
}

void IKSolver::calculateBoneLengths() {
  mNodeLengths.resize(mNodes.size());
  for (int i = 0; i < mNodes.size() - 1; ++i) {
    std::shared_ptr<GltfNode> startNode = mNodes.at(i);
    std::shared_ptr<GltfNode> endNode = mNodes.at(i + 1);

    glm::vec3 startNodePos = startNode->getGlobalPosition();
    glm::vec3 endNodePos = endNode->getGlobalPosition();

    mNodeLengths.at(i) = glm::length(endNodePos - startNodePos);
    Logger::log(2, "%s: bone %i has length %f\n", __FUNCTION__, i, mNodeLengths.at(i));
  }
}

std::shared_ptr<GltfNode> IKSolver::getIkChainRootNode() {
  return mNodes.at(mNodes.size() - 1);
}

bool IKSolver::solveCCD(const glm::vec3 target) {
  /* no nodes, no solving possible */
  if (!mNodes.size()) {
    return false;
  }

  for (unsigned int i = 0; i < mIterations; ++i) {
    /* we are really close to the target, stop iterations */
    glm::vec3 effector = mNodes.at(0)->getGlobalPosition();
    if (glm::length(target - effector) < mThreshold) {
      return true;
    }

    /* iterate the IK chain from node after effector to the root node */
    for (size_t j = 1; j < mNodes.size(); ++j) {
      std::shared_ptr<GltfNode> node = mNodes.at(j);
      if (!node) {
        Logger::log(1, "%s error: node at pos %i is invalid, skipping\n", __FUNCTION__, j);
        continue;
      }

      /* evaluate effector at start of every iteration again */
      effector = mNodes.at(0)->getGlobalPosition();
      if (glm::length(target - effector) < mThreshold) {
        return true;
      }

      /* get the global position and rotation of the node, NOT the local */
      glm::vec3 position = node->getGlobalPosition();
      glm::quat rotation = node->getGlobalRotation();

      /* create vec3 from current world position to:
       * - effector
       * - target
       * to calculate the angle we have to rotate the node about */
      glm::vec3 toEffector = glm::normalize(effector - position);
      glm::vec3 toTarget = glm::normalize(target - position);

      glm::quat effectorToTarget = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
      if (glm::length(toTarget) > mThreshold) {
        effectorToTarget = glm::rotation(toEffector, toTarget);
      }

      /* calculate the required local rotation from the world rotation */
      glm::quat worldRotation = rotation * effectorToTarget;
      glm::quat localRotation = worldRotation * glm::conjugate(rotation);

      /* rotate the node around the old plus the new rotation */
      glm::quat currentRotation = node->getLocalRotation();
      node->blendRotation(currentRotation * localRotation, 1.0f);

      /* update the node matrices, current node to effector
         to reflect the local changes down the chain */
      node->updateNodeAndChildMatrices();
    }
  }
  return true;
}
