#pragma once

struct GLFWwindow;

class InputManager
{
public:
	void initialize(GLFWwindow* inWindow);
	void processInput(GLFWwindow* inWindow);
	void resetInputs();

public:
	// todo @ech : move when proper callbacks for events are implemented
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