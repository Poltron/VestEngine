#version 450

layout (set = 1, binding = 0) uniform texture2D textures[16];

layout (set = 1, binding = 1) uniform sampler textureSampler;
	
layout (set = 2, binding = 0) uniform MaterialData
{
	vec4 color;
	uint diffuseTexIndex;
} materialData;

layout(location = 0) in vec3 inFragNormal;
layout(location = 1) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec4 texColor = texture(sampler2D(textures[materialData.diffuseTexIndex], textureSampler), inFragTexCoord);
	outColor = materialData.color * vec4(texColor.rgb, 1.0);
}