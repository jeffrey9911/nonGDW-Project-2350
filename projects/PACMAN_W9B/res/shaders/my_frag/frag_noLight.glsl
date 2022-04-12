#version 430

#include "../fragments/fs_common_inputs.glsl"

// We output a single color to the color buffer
layout(location = 0) out vec4 frag_color;

////////////////////////////////////////////////////////////////
/////////////// Instance Level Uniforms ////////////////////////
////////////////////////////////////////////////////////////////

// Represents a collection of attributes that would define a material
// For instance, you can think of this like material settings in 
// Unity
struct Material {
	sampler2D Diffuse;
	sampler2D Specular;
	float Shininess;
};
// Create a uniform for the material
uniform Material u_Material;

////////////////////////////////////////////////////////////////
///////////// Application Level Uniforms ///////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/multiple_point_lights.glsl"

////////////////////////////////////////////////////////////////
/////////////// Frame Level Uniforms ///////////////////////////
////////////////////////////////////////////////////////////////

#include "../fragments/frame_uniforms.glsl"
#include "../fragments/color_correction.glsl"

//https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {

	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	frag_color = textureColor;
}