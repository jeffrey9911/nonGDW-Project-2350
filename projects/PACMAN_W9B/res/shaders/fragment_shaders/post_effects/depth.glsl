#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec3 outColor;

uniform layout(binding = 0) sampler2D s_Image;
uniform layout(binding = 1) sampler2D s_Depth; // get depth buffer from g-buffer ( also these layout are all get from g-buffers
                                                        // which you can talk about when you record the gbuffer part

uniform float direction = 16.0;
uniform float quality = 3.0;
uniform float size = 8.0;

uniform float position = 0.0;
uniform vec2 resolution = vec2(1920, 1080);

float Pi = 6.28318530718; // pi * 2

void main() {
    float depth = texture(s_Depth, inUV).r; // get the depth data
    if(position >= (depth + 0.01) || position <= (depth - 0.01)) // if the depth is not approximately equal position =>
    {                                                                       // create a blur
        vec2 radius = size / resolution.xy;
        vec4 color = texture(s_Image, inUV);

        for( float d=0.0; d<Pi; d+=Pi/direction) 
        {
		    for(float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
            {
			    color += texture(s_Image, inUV + vec2(cos(d), sin(d)) * radius * i);		
            }
        }
        color /= quality * direction - 15.0;
        outColor = color.xyz;
    }
    else
    {
        outColor = texture(s_Image, inUV).xyz; // if the depth is approximately equal to position => do no blur
    }
}