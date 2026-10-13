#version 330 core

out vec4 FragColor;

struct Material
{
	vec3 objectColor;
};

uniform Material material;

void main()
{
	FragColor = vec4(material.objectColor, 1.0);
}