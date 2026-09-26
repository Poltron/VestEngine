#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aSize;

out vec3 vColor;

uniform mat4 uViewProjection;

void main()
{
	gl_PointSize = aSize;
	gl_Position = uViewProjection * vec4(aPos, 1.0);

	vColor = aColor;
}