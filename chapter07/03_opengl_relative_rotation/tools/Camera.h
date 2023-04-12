/* Simple camera object */
#pragma once

#include <glm/glm.hpp>

#include "OGLRenderData.h"

class Camera {
  public:
    glm::mat4 getViewMatrix(OGLRenderData &renderData);

  private:
    glm::vec3 mWorldPos = glm::vec3(-0.15f, 2.25f, 4.5f);

    glm::vec3 mViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mRightDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mUpDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    /* world up is positive Y */
    glm::vec3 mWorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
};
