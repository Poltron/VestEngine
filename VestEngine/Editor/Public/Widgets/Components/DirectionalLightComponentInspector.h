#pragma once

#include "glm/vec3.hpp"
#include "Widgets/InspectorElement.h"

class DirectionalLightComponentInspector : public IInspectorElement
{
	glm::vec3 color = glm::vec3(1.0f);
	float intensity = 1.0f;

public:
	DirectionalLightComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};