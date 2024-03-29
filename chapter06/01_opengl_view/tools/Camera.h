/* Simple camera object */
#pragma once

#include <glm/glm.hpp>

#include "OGLRenderData.h"

class Camera {
  public:
    glm::mat4 getViewMatrix(OGLRenderData &renderData);

  private:
    glm::vec3 mWorldPos = glm::vec3(0.5f, 0.25f, 1.0f);

    glm::vec3 mViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    /* world up is positive Y */
    glm::vec3 mWorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
};
