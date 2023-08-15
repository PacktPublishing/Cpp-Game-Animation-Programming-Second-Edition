#include <glm/gtx/quaternion.hpp>

#include "IKSolver.h"
#include "Logger.h"

IKSolver::IKSolver() : IKSolver(10) {}

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
  mFABRIKNodePositions.resize(mNodes.size());
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

      /* get the global position and rotation of the node, NOT the local */
      glm::vec3 position = node->getGlobalPosition();
      glm::quat rotation = node->getGlobalRotation();

      /* create normalized vec3 from current world position to:
       * - effector
       * - target
       * and calculate the angle we have to rotate the node about */
      glm::vec3 toEffector = glm::normalize(effector - position);
      glm::vec3 toTarget = glm::normalize(target - position);

      glm::quat effectorToTarget = glm::rotation(toEffector, toTarget);

      /* calculate the required local rotation from the world rotation */
      glm::quat localRotation = rotation * effectorToTarget * glm::conjugate(rotation);

      /* rotate the node LOCALLY around the old plus the new rotation */
      glm::quat currentRotation = node->getLocalRotation();
      node->blendRotation(currentRotation * localRotation, 1.0f);

      /* update the node matrices, current node to effector
         to reflect the local changes down the chain */
      node->updateNodeAndChildMatrices();

      /* evaluate effector at the end of every iteration again */
      effector = mNodes.at(0)->getGlobalPosition();
      if (glm::length(target - effector) < mThreshold) {
        return true;
      }
    }
  }

  return false;
}

/* move bones forward, closer to target */
void IKSolver::solveFABRIKForward(glm::vec3 target) {
  /* set effector to target */
  mFABRIKNodePositions.at(0) = target;

  for (size_t i = 1; i < mFABRIKNodePositions.size(); ++i) {
    glm::vec3 boneDirection =
      glm::normalize(mFABRIKNodePositions.at(i) - mFABRIKNodePositions.at(i - 1));
    glm::vec3 offset = boneDirection * mNodeLengths.at(i - 1);

    mFABRIKNodePositions.at(i) = mFABRIKNodePositions.at(i - 1) + offset;
  }
}

/* move bones backward, back to reach base */
void IKSolver::solveFABRIKBackward(glm::vec3 base) {
  /* set root node back to (saved) base */
  mFABRIKNodePositions.at(mFABRIKNodePositions.size() - 1) = base;

  for (int i = mFABRIKNodePositions.size() - 2; i >= 0; --i) {
    glm::vec3 boneDirection =
      glm::normalize(mFABRIKNodePositions.at(i) - mFABRIKNodePositions.at(i + 1));
    glm::vec3 offset = boneDirection * mNodeLengths.at(i);

    mFABRIKNodePositions.at(i) = mFABRIKNodePositions.at(i + 1) + offset;
  }
}

/* we need to ROTATE the bones, starting with the root node */
void IKSolver::adjustFABRIKNodes() {
  for (size_t i = mFABRIKNodePositions.size() - 1; i > 0; --i) {
    std::shared_ptr<GltfNode> node = mNodes.at(i);
    std::shared_ptr<GltfNode> nextNode = mNodes.at(i - 1);

    /* get the global position and rotation of the original nodes */
    glm::vec3 position = node->getGlobalPosition();
    glm::quat rotation = node->getGlobalRotation();

    /* calculate the vector of the original node direction */
    glm::vec3 nextPosition = nextNode->getGlobalPosition();
    glm::vec3 toNext = glm::normalize(nextPosition - position);

    /* calculate the vector of the changed node direction */
    glm::vec3 toDesired =
      glm::normalize(mFABRIKNodePositions.at(i - 1) - mFABRIKNodePositions.at(i));

    /* calculate the angle we have to rotate the node about */
    glm::quat nodeRotation = glm::rotation(toNext, toDesired);

    /* calculate the required local rotation from the world rotation */
    glm::quat localRotation = rotation * nodeRotation * glm::conjugate(rotation);

    /* rotate the node around the old plus the new rotation */
    glm::quat currentRotation = node->getLocalRotation();
    node->blendRotation(currentRotation * localRotation, 1.0f);

    /* update the node matrices, current node to effector
       to reflect the local changes down the chain */
    node->updateNodeAndChildMatrices();
  }
}

bool IKSolver::solveFABRIK(glm::vec3 target) {
  /* no nodes, no solving possible */
  if (!mNodes.size()) {
    return false;
  }

  /* copy node positions, we will work on the copy */
  for (size_t i = 0; i < mNodes.size(); ++i) {
    std::shared_ptr<GltfNode> node = mNodes.at(i);
    mFABRIKNodePositions.at(i) = node->getGlobalPosition();
  }

  /* get original root node position before altering the bones */
  glm::vec3 base = getIkChainRootNode()->getGlobalPosition();

  for (unsigned int i = 0; i < mIterations; ++i) {
    /* we are really close to the target, stop iterations */
    glm::vec3 effector = mFABRIKNodePositions.at(0);
    if (glm::length(target - effector) < mThreshold) {
      adjustFABRIKNodes();
      return true;
    }

    /* the solving itself */
    solveFABRIKForward(target);
    solveFABRIKBackward(base);
  }

  adjustFABRIKNodes();

  /* return true if we are close to the target */
  glm::vec3 effector = mNodes.at(0)->getGlobalPosition();
  if (glm::length(target - effector) < mThreshold) {
    return true;
  }

  return false;
}
