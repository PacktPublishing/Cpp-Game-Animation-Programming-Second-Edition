#version 460 core
layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texCoord;

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 0) uniform sampler2D tex;

vec3 lightPos = vec3(4.0, 5.0, -3.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

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
  float lightAngle = max(dot(normalize(normal), normalize(lightPos)), 0.0);
  FragColor = texture(tex, texCoord) * vec4((0.3 + 0.7 * lightAngle) * lightColor, 1.0);
  FragColor.rgb = sRGB(FragColor.rgb);
}
