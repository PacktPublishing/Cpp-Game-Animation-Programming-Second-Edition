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

  /* calculate strafe direction */
  mViewRightVector = glm::normalize(glm::cross(mWorldUpVector, mViewDirection));
  mViewUpVector = glm::normalize(glm::cross(mViewDirection, mViewRightVector));
  mStrafeViewDirection = glm::normalize(glm::cross(mViewDirection, mViewUpVector));

  /* update camera position depending on desired movement */
  mWorldPos += renderData.rdMoveForward * renderData.rdTickDiff * mViewDirection
            +  renderData.rdMoveStrafe * renderData.rdTickDiff * mStrafeViewDirection
            +  renderData.rdMoveUpDown * renderData.rdTickDiff * mViewUpVector;

  /* update in render data for UI */
  renderData.rdCameraWorldPosition = mWorldPos;

  return glm::lookAt(mWorldPos, mWorldPos + mViewDirection, mViewUpVector);
}
