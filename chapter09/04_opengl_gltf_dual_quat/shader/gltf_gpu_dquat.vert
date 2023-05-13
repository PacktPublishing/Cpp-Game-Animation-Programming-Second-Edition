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

layout (std430, binding = 2) readonly buffer JointDualQuats {
  mat2x4 jointDQs[];
};

mat2x4 getJointTransform(ivec4 joints, vec4 weights) {
  // read dual quaterions from buffer
  mat2x4 dq0 = jointDQs[joints.x];
  mat2x4 dq1 = jointDQs[joints.y];
  mat2x4 dq2 = jointDQs[joints.z];
  mat2x4 dq3 = jointDQs[joints.w];

  // shortest rotation
  weights.y *= sign(dot(dq0[0], dq1[0]));
  weights.z *= sign(dot(dq0[0], dq2[0]));
  weights.w *= sign(dot(dq0[0], dq3[0]));

  // blend
  mat2x4 result =
      weights.x * dq0 +
      weights.y * dq1 +
      weights.z * dq2 +
      weights.w * dq3;

  // normalize rotation quaternion
  float norm = length(result[0]);
  return result / norm;
}

mat4 skinMat() {
  mat2x4 bone = getJointTransform(ivec4(aJointNum), aJointWeight);

  vec4 r = bone[0]; // rotation
  vec4 t = bone[1]; // translation

  return mat4(
      1.0 - (2.0 * r.y * r.y) - (2.0 * r.z * r.z),
            (2.0 * r.x * r.y) + (2.0 * r.w * r.z),
            (2.0 * r.x * r.z) - (2.0 * r.w * r.y),
      0.0,

            (2.0 * r.x * r.y) - (2.0 * r.w * r.z),
      1.0 - (2.0 * r.x * r.x) - (2.0 * r.z * r.z),
            (2.0 * r.y * r.z) + (2.0 * r.w * r.x),
      0.0,

            (2.0 * r.x * r.z) + (2.0 * r.w * r.y),
            (2.0 * r.y * r.z) - (2.0 * r.w * r.x),
      1.0 - (2.0 * r.x * r.x) - (2.0 * r.y * r.y),
      0.0,

      2.0 * (-t.w * r.x + t.x * r.w - t.y * r.z + t.z * r.y),
      2.0 * (-t.w * r.y + t.x * r.z + t.y * r.w - t.z * r.x),
      2.0 * (-t.w * r.z - t.x * r.y + t.y * r.x + t.z * r.w),
      1);
}

void main() {
  gl_Position = projection * view * skinMat() * vec4(aPos, 1.0);
  normal = aNormal;
  texCoord = aTexCoord;
}

