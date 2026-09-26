#pragma once
#include "glm/glm.hpp"

#include "Platform/Input.h"

struct Plane
{
	glm::vec3 origin = { 0, 0, 0 };
	glm::vec3 normal = { 0, 1, 0 };
};

struct Frustum
{
	Plane top;
	Plane bottom;
	Plane left;
	Plane right;
	Plane far;
	Plane near;
};

class Camera
{
public:
	Camera();

public:
	void initialize();
	void onCameraSelected(bool bInSelected);

	void update(double inDeltaTime);

	const glm::vec3& getRotation() const;
	void setRotation(float inYaw, float inPitch);

	const glm::vec3& getPosition() const;
	void setPosition(const glm::vec3& inPosition);

	void setFOV(float inFov);

	glm::mat4& getProjectionMatrix();
	glm::mat4& getViewMatrix();
	glm::mat4& getViewProjectionMatrix();
	const Frustum& getFrustum() const;

private:
	void updateProjectionMatrix();
	void updateViewMatrix();
	void updateFrustum();

private:
	int width = 800;
	int height = 600;
	float aspectRatio = 4.0f / 3.0f;
	float near = 0.1f;
	float far = 100.0f;
	Frustum frustum;

	float fov = 45.0f;
	const float minFov = 1.0f;
	const float maxFov = 120.0f;
	float moveSpeed = 25.0f;
	float rotationSpeed = 25.0f;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
	
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 viewProjection;

	bool bDirty = false;

// inputs
private:
	void bindInputs();
	void unbindInputs();

	void onMouseMoved(double inX, double inY, double inDeltaTime);

	// bug : weird inputs sometime when scrolling just one step ???
	void onMouseScrolled(double inX, double inY, double inDeltaTime);

	// todo : handle this as an axis
	void onKeyUpPressed(input::EInputState inState, input::EKeyModifier inModifiers);
	void onKeyDownPressed(input::EInputState inState, input::EKeyModifier inModifiers);

	// todo : handle this as an axis
	void onKeyLeftPressed(input::EInputState inState, input::EKeyModifier inModifiers);
	void onKeyRightPressed(input::EInputState inState, input::EKeyModifier inModifiers);

	void consumeMouseMovementInputs(float inXOffset, float inYOffset, double inDeltaTime);
	void consumeMouseScrollInputs(float inXOffset, float inYOffset, double inDeltaTime);
	void consumeKeyboardInputs(float inHorizontalAxis, float inVerticalAxis, double inDeltaTime);

private:
	input::InputCallbackHandle mouseMoveHandle;
	input::InputCallbackHandle mouseScrollHandle;
	input::InputCallbackHandle mouseClickHandle;
	input::InputCallbackHandle keyUpHandle;
	input::InputCallbackHandle keyDownHandle;
	input::InputCallbackHandle keyLeftHandle;
	input::InputCallbackHandle keyRightHandle;

	float verticalAxis = 0.0f;
	float horizontalAxis = 0.0f;

	const float mouseSensitivity = 10.0f;
	float lastXPos = -1.0f;
	float xOffset = 0.0f;
	float lastYPos = -1.0f;
	float yOffset = 0.0f;

	const float scrollSensitivity = 100.0f;
	float scrollOffset = 0.0f;
};