#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "Components/Entity.h"

// GLM axis
// >> Forward = -Z
// >> Right = +X
// >> Up = +Y

struct LocalTransformComponent
{
private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	bool bDirty = false;

public:
	Entity entity = 0;

	LocalTransformComponent()
	{
		entity = 0;

		position = glm::vec3();
		rotation = glm::vec3();
		scale = glm::vec3(1, 1, 1);
	}

	const glm::vec3& getPosition() const { return position; }
	void setLocalPosition(const glm::vec3& inNewPosition)
	{
		position = inNewPosition;
		bDirty = true;
	}

	const glm::vec3& getRotation() const { return rotation; }
	void setLocalRotation(const glm::vec3& inNewRotation)
	{
		rotation = inNewRotation;
		bDirty = true;
	}

	void setLocalRotation(const glm::quat& inNewRotation)
	{
		rotation = glm::eulerAngles(inNewRotation);
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

		model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
		model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
		model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));

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
};
