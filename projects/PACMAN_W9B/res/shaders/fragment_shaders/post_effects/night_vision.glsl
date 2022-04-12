#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

uniform layout(binding = 0) sampler2D s_Image;

uniform vec3 u_nvVec = vec3(0.8, 0.3, 0.1);

void main() {
    vec4 color;
    color = texture(s_Image, inUV);
    float u_lum = dot(color.rgb, u_nvVec);
    color.rgb = vec3(0.0, u_lum, 0.0);
    outColor = color;
}
