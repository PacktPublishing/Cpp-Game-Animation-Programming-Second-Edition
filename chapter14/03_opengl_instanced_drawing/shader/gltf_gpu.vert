#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aJointNum;
layout (location = 4) in vec4 aJointWeight;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texCoord;

layout (std140, binding = 0) uniform Matrices {
  mat4 view;
  mat4 projection;
};

layout (std430, binding = 1) readonly buffer JointMatrices {
  mat4 jointMat[];
};

uniform int aModelStride;

void main() {
  mat4 skinMat =
    aJointWeight.x * jointMat[int(aJointNum.x) + gl_InstanceID * aModelStride] +
    aJointWeight.y * jointMat[int(aJointNum.y) + gl_InstanceID * aModelStride] +
	  aJointWeight.z * jointMat[int(aJointNum.z) + gl_InstanceID * aModelStride] +
    aJointWeight.w * jointMat[int(aJointNum.w) + gl_InstanceID * aModelStride];

  gl_Position = projection * view * skinMat * vec4(aPos, 1.0);
  normal = vec3(transpose(inverse(skinMat)) * vec4(aNormal, 1.0));
  texCoord = aTexCoord;
}
