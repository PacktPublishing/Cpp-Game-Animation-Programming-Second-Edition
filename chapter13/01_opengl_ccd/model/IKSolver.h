/* CCD and FABRIK IK solver */
#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "GltfNode.h"

class IKSolver {
  public:
    IKSolver();
    IKSolver(unsigned int iterations);
    void setNodes(std::vector<std::shared_ptr<GltfNode>> nodes);
    std::shared_ptr<GltfNode> getIkChainRootNode();

    void setNumIterations(unsigned int iterations);

    bool solveCCD(glm::vec3 target);

  private:
    /* nodes from effector (at index 0) to IK chain root node (last index) */
    std::vector<std::shared_ptr<GltfNode>> mNodes{};

    unsigned int mIterations = 0;
    float mThreshold = 0.00001f;
};
