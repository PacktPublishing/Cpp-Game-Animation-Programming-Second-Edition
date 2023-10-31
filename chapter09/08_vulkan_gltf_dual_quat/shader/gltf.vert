#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texCoord;

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

void main() {
  gl_Position = projection * view * vec4(aPos, 1.0);
  normal = aNormal;
  texCoord = aTexCoord;
}

