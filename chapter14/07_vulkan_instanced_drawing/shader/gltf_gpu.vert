#version 460 core
#extension GL_EXT_scalar_block_layout : enable
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in uvec4 aJointNum;
layout (location = 4) in vec4 aJointWeight;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texCoord;

layout (push_constant) uniform Constants {
  int aModelStride;
};

layout (set = 1, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
};

layout (std430, set = 2, binding = 0) readonly buffer JointMatrices {
    mat4 jointMat[];
};

void main() {
  mat4 skinMat =
		aJointWeight.x * jointMat[aJointNum.x + gl_InstanceIndex * aModelStride] +
		aJointWeight.y * jointMat[aJointNum.y + gl_InstanceIndex * aModelStride] +
		aJointWeight.z * jointMat[aJointNum.z + gl_InstanceIndex * aModelStride] +
		aJointWeight.w * jointMat[aJointNum.w + gl_InstanceIndex * aModelStride];
  gl_Position = projection * view * skinMat * vec4(aPos, 1.0);
  normal = vec3(transpose(inverse(skinMat)) * vec4(aNormal, 1.0));
  texCoord = aTexCoord;
}

