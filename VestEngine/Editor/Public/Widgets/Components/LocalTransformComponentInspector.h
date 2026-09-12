#pragma once

#include "glm/vec3.hpp"
#include "Widgets/InspectorElement.h"

class LocalTransformComponentInspector : public IInspectorElement
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

public:
	LocalTransformComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};