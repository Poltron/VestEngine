#pragma once

#include "glm/glm.hpp"

#include "Core/ResourceHandle.h"

struct PointRendererComponent
{
	Entity entity;

	glm::vec3 position;
	unsigned int size;
	glm::vec3 color;
};

struct LineRendererComponent
{
	Entity entity;

	glm::vec3 start;
	glm::vec3 end;
	glm::vec3 color;
};

struct SphereRendererComponent
{
	Entity entity;

	glm::vec3 radius;
	glm::vec3 color;
};

struct BoxRendererComponent
{
	Entity entity;

	glm::vec3 size;
	glm::vec3 color;
};