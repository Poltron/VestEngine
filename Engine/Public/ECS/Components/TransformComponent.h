#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "ECS/Entity.h"

// GLM axis
// >> Forward = -Z
// >> Right = +X
// >> Up = +Y

struct LocalTransformComponent
{
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	bool bDirty = false;

public:
	Entity entity = 0;

	LocalTransformComponent()
	{
		entity = 0;

		position = glm::vec3();
		rotation = glm::quat();
		scale = glm::vec3(1, 1, 1);
	}

	const glm::vec3& getPosition() const { return position; }
	void setLocalPosition(const glm::vec3& inNewPosition)
	{
		position = inNewPosition;
		bDirty = true;
	}

	const glm::quat& getRotation() const { return rotation; }
	void setLocalRotation(const glm::quat& inNewRotation)
	{
		rotation = inNewRotation;
		bDirty = true;
	}

	const glm::vec3& getScale() const { return scale; }
	void setLocalScale(const glm::vec3& inNewScale)
	{
		scale = inNewScale;
		bDirty = true;
	}

	bool isDirty() const { return bDirty; }
	void setDirty(bool inDirty)
	{ 
		bDirty = inDirty; 
	}

	glm::mat4 getLocalModelMatrix() const
	{
		glm::mat4 model = glm::translate(glm::mat4(1.0), position);
		model = model * glm::mat4(rotation);
		model = glm::scale(model, scale);

		return model;
	}
};

struct WorldTransformComponent
{
	Entity entity = 0;

	glm::mat4 model;

	WorldTransformComponent()
	{
		entity = 0;
		model = glm::identity<glm::mat4>();
	}

	glm::vec3 getModelForward() const
	{
		return glm::normalize(glm::vec3(model[2].x, model[2].y, model[2].z));
	}

	glm::vec3 getPosition() const
	{
		return { model[3][0], model[3][1], model[3][2] };
	}

	glm::vec3 getScale() const
	{
		float scaleX = glm::length(glm::vec3(model[0][0], model[0][1], model[0][2]));
		float scaleY = glm::length(glm::vec3(model[1][0], model[1][1], model[1][2]));
		float scaleZ = glm::length(glm::vec3(model[2][0], model[2][1], model[2][2]));
		return { scaleX, scaleY, scaleZ };
	}

	glm::vec3 getSquaredScale() const
	{
		float scaleX = model[0][0] * model[0][0] + model[0][1] * model[0][1] + model[0][2] * model[0][2];
		float scaleY = model[1][0] * model[1][0] + model[1][1] * model[1][1] + model[1][2] * model[1][2];
		float scaleZ = model[2][0] * model[2][0] + model[2][1] * model[2][1] + model[2][2] * model[2][2];
		return { scaleX, scaleY, scaleZ };
	}
};
