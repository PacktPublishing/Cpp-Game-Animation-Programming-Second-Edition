#version 460 core
layout (location = 0) in vec4 lineColor;
layout (location = 1) in vec2 texCoord; // ignored

layout (location = 0) out vec4 FragColor;

float toSRGB(float x) {
if (x <= 0.0031308)
        return 12.92 * x;
    else
        return 1.055 * pow(x, (1.0/2.4)) - 0.055;
}
vec3 sRGB(vec3 c) {
    return vec3(toSRGB(c.x), toSRGB(c.y), toSRGB(c.z));
}

void main() {
  FragColor = vec4(sRGB(lineColor.rgb), lineColor.a);
}
