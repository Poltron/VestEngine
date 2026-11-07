#version 450

struct AmbientLight
{
	vec3 color;
	float intensity;
};

struct DirectionalLight
{
	vec3 color;
	float intensity;
	vec3 forward;
};

struct PointLight
{
	vec3 color;
	float intensity;
	vec3 position;
    float constant;
    float linear;
    float quadratic; 
	
	// explicit padding
	float padding1;
	float padding2;
};

layout (set = 0, binding = 1) uniform LightData
{
	AmbientLight ambient;
	DirectionalLight directional;
	PointLight points[4];
	uint pointLightCount;
	vec3 viewPosition;
} lightData;

layout (set = 1, binding = 0) uniform texture2D textures[16];

layout (set = 1, binding = 1) uniform sampler textureSampler;
	
layout (set = 2, binding = 0) uniform MaterialData
{
	vec4 color;
	uint diffuseTexIndex;
	uint specularTexIndex;
	float shininess;
} materialData;

layout(location = 0) in vec3 inFragPosition;
layout(location = 1) in vec3 inFragNormal;
layout(location = 2) in vec2 inFragTexCoord;

layout(location = 0) out vec4 outColor;

//void main() 
//{
//	vec4 texColor = texture(sampler2D(textures[materialData.diffuseTexIndex], textureSampler), inFragTexCoord);
//	outColor = materialData.color * vec4(texColor.rgb, 1.0);
//}

vec3 computeDirectionalLight(DirectionalLight directional, vec3 normal, vec3 viewDirection, vec3 diffuseTexValue, vec3 specularTexValue)
{
	vec3 directionalValue = directional.color * directional.intensity;
	vec3 lightDirection = normalize(-directional.forward);
	
	float diffRatio = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = directionalValue * diffRatio * diffuseTexValue;

	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specRatio = pow(max(dot(viewDirection, reflectDirection), 0.0), materialData.shininess);
	vec3 specular = directionalValue * specRatio * specularTexValue;
	
	return (diffuse + specular);
}

vec3 computePointLight(PointLight point, vec3 normal, vec3 viewDirection, vec3 fragPosition, vec3 diffuseTexValue, vec3 specularTexValue)
{
	vec3 pointValue = point.color * point.intensity;
	vec3 lightDirection = normalize(point.position - fragPosition);
    float distance = length(point.position - fragPosition);
    float attenuation = 1.0 / (point.constant + point.linear * distance + point.quadratic * (distance * distance));  
	
	float diffRatio = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = pointValue * diffRatio * attenuation * diffuseTexValue;
	
	vec3 reflectDirection = reflect(-lightDirection, normal);
	float specRatio = pow(max(dot(viewDirection, reflectDirection), 0.0), materialData.shininess);
	vec3 specular = pointValue * specRatio * attenuation * specularTexValue;
	
	return (diffuse + specular);
}

void main()
{
	vec3 normal = normalize(inFragNormal);
	vec3 viewDirection = normalize(lightData.viewPosition - inFragPosition);

	vec3 diffuseTexValue = vec3(texture(sampler2D(textures[materialData.diffuseTexIndex], textureSampler), inFragTexCoord));
	vec3 specularTexValue = vec3(texture(sampler2D(textures[materialData.specularTexIndex], textureSampler), inFragTexCoord));

	vec3 ambientValue = lightData.ambient.color * lightData.ambient.intensity;
	vec3 finalColor = ambientValue * diffuseTexValue;
	finalColor += ambientValue * specularTexValue;

	finalColor += computeDirectionalLight(lightData.directional, normal, viewDirection, diffuseTexValue, specularTexValue);

	for (int i = 0; i < lightData.pointLightCount; i++)
	{
		finalColor += computePointLight(lightData.points[i], normal, viewDirection, inFragPosition, diffuseTexValue, specularTexValue);
	}

	outColor = vec4(finalColor, 1.0) * materialData.color;
}