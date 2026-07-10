#version 330 core

//
in vec3 FragPosition;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

//
struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPosition;

void main()
{
	vec3 diffuseTexValue = vec3(texture(material.diffuse, TexCoords));
	vec3 specularTexValue = vec3(texture(material.specular, TexCoords));

	// --- ambient
	vec3 ambient = light.ambient * diffuseTexValue;
	
	// --- diffuse
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(light.position - FragPosition);
	float diffuseDot = max(dot(normal, lightDirection), 0.0);

	vec3 diffuse = light.diffuse * diffuseDot * diffuseTexValue;
	
	// --- specular
	vec3 viewDirection = normalize(viewPosition - FragPosition);
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specularDot =	max(dot(viewDirection, reflectDirection), 0.0);
	float specularPower = pow(specularDot, material.shininess);

	vec3 specular = light.specular * specularPower * specularTexValue;
	
	// --- results
	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}