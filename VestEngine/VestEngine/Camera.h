#pragma once
#include "glm/glm.hpp"

class Camera
{
public:
	Camera();

public:
	const glm::vec3& getRotation() const;
	void setRotation(float inYaw, float inPitch);

	const glm::vec3& getPosition() const;
	void setPosition(const glm::vec3& inPosition);

	void setFOV(float inFov);

	glm::mat4& getProjectionMatrix();
	glm::mat4& getViewMatrix();

public:
	void consumeMouseMovementInputs(float inXOffset, float inYOffset, double inDeltaTime);
	void consumeMouseScrollInputs(float inScrollOffset, double inDeltaTime);
	void consumeKeyboardInputs(float inHorizontalAxis, float inVerticalAxis, double inDeltaTime);

private:
	void updateProjectionMatrix();
	void updateViewMatrix();

private:
	int width = 800;
	int height = 600;
	float aspectRatio = 4.0f / 3.0f;
	float near = 0.1f;
	float far = 100.0f;

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
};