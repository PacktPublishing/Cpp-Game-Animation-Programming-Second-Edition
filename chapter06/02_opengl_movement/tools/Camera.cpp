#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Logger.h"

glm::mat4 Camera::getViewMatrix(OGLRenderData &renderData) {
  float azimRad = glm::radians(renderData.rdViewAzimuth);
  float elevRad = glm::radians(renderData.rdViewElevation);

  float sinAzim = glm::sin(azimRad);
  float cosAzim = glm::cos(azimRad);
  float sinElev = glm::sin(elevRad);
  float cosElev = glm::cos(elevRad);

  /* update view direction */
   mViewDirection = glm::normalize(glm::vec3(
     sinAzim * cosElev, sinElev, -cosAzim * cosElev));

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
