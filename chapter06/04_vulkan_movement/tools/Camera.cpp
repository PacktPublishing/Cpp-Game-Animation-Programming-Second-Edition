#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

glm::mat4 Camera::getViewMatrix(VkRenderData &renderData) {
  float azimRad = glm::radians(renderData.rdViewAzimuth);
  float elevRad = glm::radians(renderData.rdViewElevation);

  float sinAzim = glm::sin(azimRad);
  float cosAzim = glm::cos(azimRad);
  float sinElev = glm::sin(elevRad);
  float cosElev = glm::cos(elevRad);

  /* update view direction */
  mViewDirection = glm::normalize(glm::vec3(
     sinAzim * cosElev, -sinElev, -cosAzim * cosElev));

  /* calculate right and up direction */
  mRightDirection = glm::normalize(glm::cross(mViewDirection, mWorldUpVector));
  mUpDirection = glm::normalize(glm::cross(mRightDirection, mViewDirection));

  /* update camera position depending on desired movement */
  mWorldPos += renderData.rdMoveForward * renderData.rdTickDiff * mViewDirection
            +  renderData.rdMoveRight * renderData.rdTickDiff * mRightDirection
            +  renderData.rdMoveUp * renderData.rdTickDiff * mUpDirection;

  /* update in render data for UI */
  renderData.rdCameraWorldPosition = mWorldPos;

  return glm::lookAt(mWorldPos, mWorldPos + mViewDirection, mUpDirection);
}
