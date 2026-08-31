#pragma once
#include "glm/glm.hpp"

class InputManager;

struct Plane
{
	glm::vec3 normal = { 0, 1, 0 };
	float distance = 0.0f;
};

struct Frustum
{
	Plane top;
	Plane bottom;

	Plane right;
	Plane left;

	Plane far;
	Plane near;
};

class Camera
{
public:
	Camera();

public:
	void initialize(InputManager* inInputManager);
	void update(double inDeltaTime);

	const glm::vec3& getRotation() const;
	void setRotation(float inYaw, float inPitch);

	const glm::vec3& getPosition() const;
	void setPosition(const glm::vec3& inPosition);

	void setFOV(float inFov);

	glm::mat4& getProjectionMatrix();
	glm::mat4& getViewMatrix();
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
	const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);

	glm::mat4 projection;
	glm::mat4 view;

// inputs
private:
	void onMouseMoved(double inX, double inY, double inDeltaTime);

	// bug : weird inputs sometime when scrolling just one step ???
	void onMouseScrolled(double inX, double inY, double inDeltaTime);

	// todo : handle this as an axis
	void onKeyUpPressed(int inState, int inMods);
	void onKeyDownPressed(int inState, int inMods);

	// todo : handle this as an axis
	void onKeyLeftPressed(int inState, int inMods);
	void onKeyRightPressed(int inState, int inMods);

	void consumeMouseMovementInputs(float inXOffset, float inYOffset, double inDeltaTime);
	void consumeMouseScrollInputs(float inXOffset, float inYOffset, double inDeltaTime);
	void consumeKeyboardInputs(float inHorizontalAxis, float inVerticalAxis, double inDeltaTime);

private:
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