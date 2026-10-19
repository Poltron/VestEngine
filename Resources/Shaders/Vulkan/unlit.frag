#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inFragNormal;
layout(location = 1) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = vec4(texture(texSampler, inFragTexCoord).rgb, 1.0);
}