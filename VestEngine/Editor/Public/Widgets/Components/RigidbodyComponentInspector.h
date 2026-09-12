#pragma once

#include "glm/vec3.hpp"
#include "Widgets/InspectorElement.h"

class RigidbodyComponentInspector : public IInspectorElement
{
	glm::vec3 angularVelocity = glm::vec3();
	glm::vec3 linearVelocity = glm::vec3();

public:
	RigidbodyComponentInspector(Entity inEntity)
		: IInspectorElement(inEntity)
	{}

	virtual std::string getLabel() override;
	virtual void update() override;
};