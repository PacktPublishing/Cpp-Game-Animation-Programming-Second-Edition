#version 460 core
layout (location = 0) in vec4 lineColor;

out vec4 FragColor;

void main() {
  FragColor = lineColor;
}
