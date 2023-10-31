#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Logger.h"

glm::mat4 Camera::getViewMatrix(VkRenderData &renderData) {
  /* update view direction */
  mViewDirection = glm::normalize(glm::vec3(
     sin(renderData.rdViewAzimuth/180.0*M_PI) * cos(renderData.rdViewElevation/180.0*M_PI),
    -sin(renderData.rdViewElevation/180.0*M_PI),
    -cos(renderData.rdViewAzimuth/180.0*M_PI) * cos(renderData.rdViewElevation/180.0*M_PI)));

  return glm::lookAt(mWorldPos, mWorldPos + mViewDirection, mWorldUpVector);
}
