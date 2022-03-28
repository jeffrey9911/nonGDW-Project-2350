#version 410

layout(location = 0) in vec3 inColor;

out vec4 frag_color;

void main() { 
	
	frag_color = vec4(inColor, 1.0);
}