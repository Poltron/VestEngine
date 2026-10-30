#version 450

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (set = 1, binding = 1) uniform MaterialData
{
	vec4 color;
} materialData;
	
layout (push_constant) uniform FragInstanceData
{
	vec4 color;
} instanceData;

layout(location = 0) in vec3 inFragNormal;
layout(location = 1) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = materialData.color * instanceData.color * vec4(texture(texSampler, inFragTexCoord).rgb, 1.0);
}