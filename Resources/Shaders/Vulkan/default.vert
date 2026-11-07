#version 450

layout (set = 0, binding = 0) uniform CameraData
{
    mat4 view;
    mat4 proj;
} cameraData;

layout (push_constant) uniform InstanceData
{
    mat4 model;
} instanceData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outFragPosition;
layout(location = 1) out vec3 outFragNormal;
layout(location = 2) out vec2 outFragTexCoord;

void main()
{
    gl_Position = cameraData.proj * cameraData.view * instanceData.model * vec4(inPosition, 1.0);
	
	outFragPosition = vec3(instanceData.model * vec4(inPosition, 1.0));
	outFragNormal = mat3(transpose(inverse(instanceData.model))) * inNormal;
    outFragTexCoord = inTexCoord;
}