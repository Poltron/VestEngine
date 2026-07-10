#version 330 core

in vec3 FragPosition;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPosition - FragPosition);
	float diffuseDot = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diffuseDot * lightColor;
	
	float specularStrength = 0.5;
	vec3 viewDirection = normalize(viewPosition - FragPosition);
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specularDot =	max(dot(viewDirection, reflectDirection), 0.0);
	float specularPower = pow(specularDot, 32);
	vec3 specular = specularStrength * specularPower * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}