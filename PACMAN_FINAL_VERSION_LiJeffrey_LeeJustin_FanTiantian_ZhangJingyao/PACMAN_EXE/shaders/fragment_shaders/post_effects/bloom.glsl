#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec3 outColor;

uniform layout(binding = 0) sampler2D s_Image;
//uniform layout(binding = 1) sampler2D s_bright;

uniform float u_spread = 1;
uniform float u_intens = 2;


void main() {
    
    ivec2 size = textureSize(s_Image, 0); // get image from gbuffer, get the size

    float uv_x = inUV.x * size.x;
    float uv_y = inUV.y * size.y;

    vec4 sum = vec4(0.0);

    for(int n = 0; n < 9; ++n) // blur the texture by creating overlapping shadows using texelFetch
    {
        uv_y = (inUV.y * size.y) + (u_spread * float(n - 4));
        vec4 h_sum = vec4(0.0);

        h_sum += texelFetch(s_Image, ivec2(uv_x - (4.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x - (3.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x - (2.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x - (1.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x , uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x + (1.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x + (2.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x + (3.0 * u_spread), uv_y), 0);
        h_sum += texelFetch(s_Image, ivec2(uv_x + (4.0 * u_spread), uv_y), 0);
        sum += h_sum / 9.0;
    }

    outColor = (texture(s_Image, inUV) - ((sum / 9.0) * u_intens)).rgb; // make the bright part more bright
}
