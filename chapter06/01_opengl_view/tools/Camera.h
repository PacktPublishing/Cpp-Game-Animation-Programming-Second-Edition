/* Simple camera object */
#pragma once

#include <glm/glm.hpp>

#include "OGLRenderData.h"

class Camera {
  public:
    void init();

    glm::vec3 getViewDirection() const;
    glm::vec3 getStrafeDirection() const;
    glm::vec3 getUpDirection() const;

    float getOrientation() const;
    void moveOrientation(float delta);

    float getHeadViewAngle();
    void moveHeadViewAngle(float delta);

  private:
    void validateAngles();
    void recalcViewDirection();

    /* set some values */
    float mOrientation = 320.0f;
    float mHeadViewAngle = 15.0f;

    glm::vec3 mViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mStrafeViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 mViewRightVector = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mViewUpVector = glm::vec3(0.0f, 0.0f, 0.0f);

    /* world up is positive Y */
    glm::vec3 mWorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
};
