#version 330 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 FragPosition;
in vec3 Normal;

out vec4 FragColor;

uniform Material material;
uniform Light light;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPosition;

void main()
{
	// --- ambient
	vec3 ambient = light.ambient * material.ambient;
	
	// --- diffuse
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(light.position - FragPosition);
	float diffuseDot = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = light.diffuse * (diffuseDot * material.diffuse);
	
	// --- specular
	vec3 viewDirection = normalize(viewPosition - FragPosition);
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specularDot =	max(dot(viewDirection, reflectDirection), 0.0);
	float specularPower = pow(specularDot, material.shininess);
	vec3 specular = light.specular * (specularPower * material.specular);
	
	// --- results
	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}