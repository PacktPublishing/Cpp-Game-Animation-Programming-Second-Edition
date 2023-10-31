#version 460 core
layout (location = 0) in vec2 texCoord;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D Tex;

void main() {
  FragColor = texture(Tex, texCoord);
}
