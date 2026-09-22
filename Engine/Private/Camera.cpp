#include "Camera.h"

#include <functional>
#include <iostream>

#include "glm/gtc/matrix_transform.hpp"
#include "tracy/Tracy.hpp"

#include "Platform/InputManager.h"
#include "Platform/Platform.h"

namespace
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

void Camera::initialize()
{
	platform::getInputManager().registerCursorPosCallback(
		[this](double inX, double inY, double inDeltaTime)
		{
			if (platform::getInputManager().getCursorInputMode() == input::ECursorInputMode::DISABLED)
			{
				onMouseMoved(inX, inY, inDeltaTime);
			}
			else
			{
				lastXPos = -1.0f;
				lastYPos = -1.0f;
			}
		});

	platform::getInputManager().registerScrollCallback(
		[this](double inX, double inY, double inDeltaTime)
		{
			if (platform::getInputManager().getCursorInputMode() == input::ECursorInputMode::DISABLED)
			{
				onMouseScrolled(inX, inY, inDeltaTime);
			}
		});

	platform::getInputManager().registerMouseCallback(input::EMouseButton::RIGHT
		, [this](input::EInputState inState, double inDeltaTime)
		{
			if (inState == input::EInputState::PRESS)
			{
				platform::getInputManager().setCursorInputMode(input::ECursorInputMode::DISABLED);
			}
			else if (inState == input::EInputState::RELEASE)
			{
				platform::getInputManager().setCursorInputMode(input::ECursorInputMode::NORMAL);
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::UP
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			if (platform::getInputManager().getCursorInputMode() == input::ECursorInputMode::DISABLED)
			{
				onKeyUpPressed(inState, inModifiers);
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::DOWN
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			if (platform::getInputManager().getCursorInputMode() == input::ECursorInputMode::DISABLED)
			{
				onKeyDownPressed(inState, inModifiers);
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::LEFT
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			if (platform::getInputManager().getCursorInputMode() == input::ECursorInputMode::DISABLED)
			{
				onKeyLeftPressed(inState, inModifiers);
			}
		});

	platform::getInputManager().registerKeyCallback(input::EKey::RIGHT
		, [this](input::EInputState inState, input::EKeyModifier inModifiers, double inDeltaTime)
		{
			if (platform::getInputManager().getCursorInputMode() == input::ECursorInputMode::DISABLED)
			{
				onKeyRightPressed(inState, inModifiers);
			}
		});
}

void Camera::update(double inDeltaTime)
{
	ZoneScoped;

	//std::cout << "pos x: " << getPosition().x << " / y: " << getPosition().y << " / z: " << getPosition().z << std::endl;
	//std::cout << "rot x: " << getRotation().x << " / y: " << getRotation().y << " / z: " << getRotation().z << std::endl;

	consumeKeyboardInputs(horizontalAxis, verticalAxis, inDeltaTime);

	// note : should not be necessary but weird inputs ??
	//consumeMouseScrollInputs(0, scrollOffset, inDeltaTime);
	//scrollOffset = 0;

	if (bDirty)
	{
		updateProjectionMatrix();
		updateViewMatrix();

		bDirty = false;
	}
}

const glm::vec3& Camera::getRotation() const
{
	return rotation;
}

void Camera::setRotation(float inYaw, float inPitch)
{
	rotation = glm::vec3(inYaw, inPitch, rotation.z);
	bDirty = true;
}

const glm::vec3& Camera::getPosition() const
{
	return position;
}

void Camera::setPosition(const glm::vec3& inPosition)
{
	position = inPosition;
	bDirty = true;
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
	bDirty = true;
}

void Camera::consumeMouseMovementInputs(float inXOffset, float inYOffset, double inDeltaTime)
{
	if (inXOffset == 0.0f && inYOffset == 0.0f)
	{
		return;
	}

	float yaw = rotation.x + (inXOffset * (float)inDeltaTime);
	float pitch = glm::clamp(rotation.y + (inYOffset * (float)inDeltaTime), -89.0f, 89.0f);
	setRotation(yaw, pitch);
}

void Camera::consumeMouseScrollInputs(float inXOffset, float inYOffset, double inDeltaTime)
{
	if (inYOffset == 0.0f)
	{
		return;
	}

	float newFOV = fov;
	newFOV -= inYOffset * (float)inDeltaTime;
	newFOV = glm::clamp(fov, minFov, maxFov);
	setFOV(newFOV);
}

void Camera::consumeKeyboardInputs(float inHorizontalAxis, float inVerticalAxis, double inDeltaTime)
{
	float speed = moveSpeed * (float)inDeltaTime;

	if (!floatEquals(inHorizontalAxis, 0))
	{
		glm::vec3 velocity = glm::normalize(glm::cross(forward, up)) * inHorizontalAxis * speed;
		setPosition(position + velocity);
	}

	if (!floatEquals(inVerticalAxis, 0))
	{
		glm::vec3 velocity = forward * inVerticalAxis * speed;
		setPosition(position + velocity);
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

void Camera::onKeyUpPressed(input::EInputState inState, input::EKeyModifier inModifiers)
{
	if (inState == input::EInputState::PRESS)
	{
		verticalAxis = 1.0f;
	}
	else if (inState == input::EInputState::RELEASE)
	{
		verticalAxis = 0;
	}
}

void Camera::onKeyDownPressed(input::EInputState inState, input::EKeyModifier inModifiers)
{
	if (inState == input::EInputState::PRESS)
	{
		verticalAxis = -1.0f;
	}
	else if (inState == input::EInputState::RELEASE)
	{
		verticalAxis = 0;
	}
}

void Camera::onKeyLeftPressed(input::EInputState inState, input::EKeyModifier inModifiers)
{
	if (inState == input::EInputState::PRESS)
	{
		horizontalAxis = -1.0f;
	}
	else if (inState == input::EInputState::RELEASE)
	{
		horizontalAxis = 0;
	}
}

void Camera::onKeyRightPressed(input::EInputState inState, input::EKeyModifier inModifiers)
{
	if (inState == input::EInputState::PRESS)
	{
		horizontalAxis = 1.0f;
	}
	else if (inState == input::EInputState::RELEASE)
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
	float yaw = rotation.x;
	float pitch = rotation.y;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(direction);

	view = glm::lookAt(position, position + forward, up);
}

void Camera::updateFrustum()
{
	const float halfVSide = far * tanf(fov * 0.5f);
	const float halfHSide = halfVSide * aspectRatio;
	const glm::vec3 frontMultFar = far * forward;

	//frustum.near = { position + near * forward, near};
}