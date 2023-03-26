#define _USE_MATH_DEFINES
#include <cmath>

#include "Camera.h"
#include "Logger.h"

void Camera::init() {
  recalcViewDirection();
}

void Camera::moveOrientation(float delta) {
  mOrientation += delta;
  recalcViewDirection();
}

float Camera::getOrientation() const {
  return mOrientation;
}

void Camera::moveHeadViewAngle(float delta) {
  mHeadViewAngle += delta;
  recalcViewDirection();
}

float Camera::getHeadViewAngle() {
  return mHeadViewAngle;
}

void Camera::validateAngles() {
  if (mOrientation < 0.0) {
    mOrientation += 360.0;
  }

  if (mOrientation >= 360.0) {
    mOrientation -= 360.0;
  }

  if (mHeadViewAngle > 89.0) {
    mHeadViewAngle = 89.0;
  }

  if (mHeadViewAngle < -89.0) {
    mHeadViewAngle = -89.0;
  }
}

void Camera::recalcViewDirection() {
  validateAngles();

  /* full view direction */
  mViewDirection = glm::normalize(glm::vec3(
     sin(mOrientation/180.0*M_PI) * cos(mHeadViewAngle/180.0*M_PI),
    -sin(mHeadViewAngle/180.0*M_PI),
    -cos(mOrientation/180.0*M_PI) * cos(mHeadViewAngle/180.0*M_PI)));

  /* strafe direction */
  mViewRightVector = glm::normalize(glm::cross(mWorldUpVector, mViewDirection));
  mViewUpVector = glm::normalize(glm::cross(mViewDirection, mViewRightVector));
  mStrafeViewDirection = glm::normalize(glm::cross(mViewDirection, mViewUpVector));
}

glm::vec3 Camera::getViewDirection() const {
  return mViewDirection;
}

glm::vec3 Camera::getStrafeDirection() const {
  return mStrafeViewDirection;
}

glm::vec3 Camera::getUpDirection() const {
  return mViewUpVector;
}
