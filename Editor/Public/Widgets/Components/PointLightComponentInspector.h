#pragma once

#include "glm/vec3.hpp"
#include "Widgets/InspectorElement.h"

class PointLightComponentInspector : public IInspectorElement
{
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;

	float constant = 1.0f;
	float linear = 1.0f;
	float quadratic = 1.0f;

public:
	PointLightComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};