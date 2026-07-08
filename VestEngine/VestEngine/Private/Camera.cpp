#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace camera_private
{
	bool floatEquals(float a, float b)
	{
		float r = a - b;
		return std::fabs(r) < FLT_EPSILON;
	}
}

Camera::Camera()
{
	aspectRatio = (float)width / (float)height;

	updateProjectionMatrix();
	updateViewMatrix();
}

const glm::vec3& Camera::getRotation() const
{
	return rotation;
}

void Camera::setRotation(float inYaw, float inPitch)
{
	rotation = glm::vec3(inYaw, inPitch, rotation.z);
}

const glm::vec3& Camera::getPosition() const
{
	return position;
}

void Camera::setPosition(const glm::vec3& inPosition)
{
	position = inPosition;
}

glm::mat4& Camera::getProjectionMatrix()
{
	return projection;
}

glm::mat4& Camera::getViewMatrix()
{
	return view;
}

void Camera::setFOV(float inFov)
{
	fov = inFov;
	projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

void Camera::consumeMouseMovementInputs(float inXOffset, float inYOffset, double inDeltaTime)
{
	if (inXOffset == 0.0f && inYOffset == 0.0f)
	{
		return;
	}

	float yaw = rotation.x + (inXOffset * (float)inDeltaTime);
	float pitch = glm::clamp(rotation.y + (inYOffset * (float)inDeltaTime), -89.0f, 89.0f);

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(direction);

	rotation = glm::vec3(yaw, pitch, rotation.z);

	updateViewMatrix();
}

void Camera::consumeMouseScrollInputs(float inScrollOffset, double inDeltaTime)
{
	if (inScrollOffset == 0.0f)
	{
		return;
	}

	fov -= inScrollOffset * (float)inDeltaTime;
	fov = glm::clamp(fov, minFov, maxFov);

	updateProjectionMatrix();
}

void Camera::consumeKeyboardInputs(float inHorizontalAxis, float inVerticalAxis, double inDeltaTime)
{
	bool bDirty = false;
	float speed = moveSpeed * (float)inDeltaTime;

	if (!camera_private::floatEquals(inHorizontalAxis, 0))
	{
		position += glm::normalize(glm::cross(forward, up)) * inHorizontalAxis * speed;
		bDirty = true;
	}

	if (!camera_private::floatEquals(inVerticalAxis, 0))
	{
		position += forward * inVerticalAxis * speed;
		bDirty = true;
	}

	if (bDirty)
	{
		updateViewMatrix();
	}
}

void Camera::updateProjectionMatrix()
{
	projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

void Camera::updateViewMatrix()
{
	view = glm::lookAt(position, position + forward, up);
}