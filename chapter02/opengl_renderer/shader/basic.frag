#version 460 core
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D Tex;

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
  FragColor = texture(Tex, texCoord);
  FragColor.rgb = sRGB(FragColor.rgb);
}
