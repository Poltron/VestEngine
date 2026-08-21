#version 330 core

//
in vec3 FragPos;
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

struct AmbientLight
{
	vec3 color;
	float intensity;
};

struct DirectionalLight
{
	vec3 color;
	float intensity;
	vec3 direction;
};

struct PointLight
{
	vec3 color;
	float intensity;
	vec3 position;

    float constant;
    float linear;
    float quadratic; 
};

uniform Material material;

#define MAX_POINT_LIGHTS 3
uniform AmbientLight ambientLight;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

uniform vec3 viewPosition;
uniform int pointLightAmount;

vec3 computeDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
	vec3 lightDirection = normalize(-light.direction);
	float diff = max(dot(normal, lightDirection), 0.0);

	vec3 reflectDirection = reflect(-lightDirection, normal);
	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);

	vec3 diffuse = light.color * light.intensity * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.color * light.intensity * spec * vec3(texture(material.specular, TexCoords));
	return (diffuse + specular);
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 viewDirection, vec3 fragPosition)
{
	vec3 lightDirection = normalize(light.position - fragPosition);
	float diff = max(dot(normal, lightDirection), 0.0);

	vec3 reflectDirection = reflect(-lightDirection, normal);
	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
	
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  

	vec3 diffuse = light.color * light.intensity * diff * attenuation * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.color * light.intensity * spec * attenuation * vec3(texture(material.specular, TexCoords));
	
	return (diffuse + specular);
}

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDirection = normalize(viewPosition - FragPos);

	vec3 result = ambientLight.color * vec3(texture(material.diffuse, TexCoords));
	result += ambientLight.color * vec3(texture(material.specular, TexCoords));

	result += computeDirectionalLight(directionalLight, normal, viewDirection);

	for (int i = 0; i < pointLightAmount; i++)
	{
		result += computePointLight(pointLights[i], normal, viewDirection, FragPos);
	}

	FragColor = vec4(result, 1.0);
}