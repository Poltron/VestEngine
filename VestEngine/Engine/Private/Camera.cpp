#include "Camera.h"

#include <functional>

#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"

#include "Managers/InputManager.h"

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

	position = glm::vec3(0, 0, 10);

	updateProjectionMatrix();
	updateViewMatrix();
}

void Camera::initialize(InputManager* inInputManager)
{
	if (!inInputManager)
		return;

	inInputManager->registerCursorPosCallback(
		[this](double xPos, double yPos, double deltaTime)
		{
			onMouseMoved(xPos, yPos, deltaTime);
		});

	inInputManager->registerScrollCallback(
		[this](double xPos, double yPos, double deltaTime)
		{
			onMouseScrolled(xPos, yPos, deltaTime);
		});

	inInputManager->registerKeyCallback(GLFW_KEY_UP
		, [this](int inState, int inMods, double inDeltaTime)
		{
			onKeyUpPressed(inState, inMods);
		});

	inInputManager->registerKeyCallback(GLFW_KEY_DOWN
		, [this](int inState, int inMods, double inDeltaTime)
		{
			onKeyDownPressed(inState, inMods);
		});

	inInputManager->registerKeyCallback(GLFW_KEY_LEFT
		, [this](int inState, int inMods, double inDeltaTime)
		{
			onKeyLeftPressed(inState, inMods);
		});

	inInputManager->registerKeyCallback(GLFW_KEY_RIGHT
		, [this](int inState, int inMods, double inDeltaTime)
		{
			onKeyRightPressed(inState, inMods);
		});
}

void Camera::update(double inDeltaTime)
{
	consumeKeyboardInputs(horizontalAxis, verticalAxis, inDeltaTime);

	// note : should not be necessary but weird inputs ??
	consumeMouseScrollInputs(0, scrollOffset, inDeltaTime);
	scrollOffset = 0;
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

const Frustum& Camera::getFrustum() const
{
	return frustum;
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

void Camera::consumeMouseScrollInputs(float inXOffset, float inYOffset, double inDeltaTime)
{
	if (inYOffset == 0.0f)
	{
		return;
	}

	fov -= inYOffset * (float)inDeltaTime;
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

void Camera::onMouseMoved(double inX, double inY, double inDeltaTime)
{
	if (lastXPos == -1.0f && lastYPos == -1.0f)
	{
		lastXPos = (float)inX;
		lastYPos = (float)inY;
	}

	xOffset = (float)inX - lastXPos;
	yOffset = lastYPos - (float)inY;
	lastXPos = (float)inX;
	lastYPos = (float)inY;

	xOffset *= mouseSensitivity;
	yOffset *= mouseSensitivity;

	consumeMouseMovementInputs(xOffset, yOffset, inDeltaTime);
}

void Camera::onMouseScrolled(double inX, double inY, double inDeltaTime)
{
	const float adjustedY = (float)inY* scrollSensitivity;
	consumeMouseScrollInputs(0, adjustedY, inDeltaTime);
}

void Camera::onKeyUpPressed(int inState, int inMods)
{
	if (inState == GLFW_PRESS)
	{
		verticalAxis = 1.0f;
	}
	else if (inState == GLFW_RELEASE)
	{
		verticalAxis = 0;
	}
}

void Camera::onKeyDownPressed(int inState, int inMods)
{
	if (inState == GLFW_PRESS)
	{
		verticalAxis = -1.0f;
	}
	else if (inState == GLFW_RELEASE)
	{
		verticalAxis = 0;
	}
}

void Camera::onKeyLeftPressed(int inState, int inMods)
{
	if (inState == GLFW_PRESS)
	{
		horizontalAxis = -1.0f;
	}
	else if (inState == GLFW_RELEASE)
	{
		horizontalAxis = 0;
	}
}

void Camera::onKeyRightPressed(int inState, int inMods)
{
	if (inState == GLFW_PRESS)
	{
		horizontalAxis = 1.0f;
	}
	else if (inState == GLFW_RELEASE)
	{
		horizontalAxis = 0;
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

void Camera::updateFrustum()
{
	const float halfVSide = far * tanf(fov * 0.5f);
	const float halfHSide = halfVSide * aspectRatio;
	const glm::vec3 frontMultFar = far * forward;

	//frustum.near = { position + near * forward, near};
}