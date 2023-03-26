/* Dear ImGui */
#pragma once

#include "VkRenderData.h"

class UserInterface {
  public:
    bool init(VkRenderData& renderData);
    void createFrame(VkRenderData& renderData);
    void render(VkRenderData& renderData);
    void cleanup(VkRenderData& renderData);

  private:
    float framesPerSecond = 0.0f;
    /* averaging speed */
    float averagingAlpha = 0.96f;
};
