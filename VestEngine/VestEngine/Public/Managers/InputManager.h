#pragma once

#include <functional>
#include <unordered_map>

struct GLFWwindow;

struct KeyInput
{
	int key;
	int state;
	int mods;

	KeyInput(int inKey, int inState, int inMods)
		: key(inKey), state(inState), mods(inMods)
	{}
};

struct MouseInput
{
	int button;
	int state;

	MouseInput(int inButton, int inState)
		: button(inButton), state(inState)
	{}
};

struct CursorPosInput
{
	double x;
	double y;

	CursorPosInput(double inX, double inY)
		: x(inX), y(inY)
	{}
};

struct ScrollInput
{
	double x;
	double y;

	ScrollInput(double inX, double inY)
		: x(inX), y(inY)
	{}
};

class InputManager
{

public:
	void initialize(GLFWwindow* inWindow);
	void processInput(GLFWwindow* inWindow, double inDeltaTime);

public:
	using KeyCallback = std::function<void(int/* state */, int/* mods */, double/* deltaTime */)>;
	using MouseCallback = std::function<void(int /* state */, double/* deltaTime */)>;
	using CursorPosCallback = std::function<void(double/* xPos */, double/* yPos */, double/* deltaTime */)>;
	using ScrollCallback = std::function<void(double/* x */, double/* y */, double/* deltaTime */)>;

	void registerKeyCallback(int key, KeyCallback inCallback);
	void registerMouseCallback(int button, MouseCallback inCallback);
	void registerCursorPosCallback(CursorPosCallback inCallback);
	void registerScrollCallback(ScrollCallback inScrollback);

private:
	// todo : 
	// - add register / unregister methods
	std::unordered_map<int, std::vector<KeyCallback>> keyCallbacks;
	std::vector<KeyInput> keyInputs;

	std::unordered_map<int, std::vector<MouseCallback>> mouseCallbacks;
	std::vector<MouseInput> mouseInputs;

	std::vector<CursorPosCallback> cursorPosCallbacks;
	std::vector<CursorPosInput> cursorPosInputs;

	std::vector<ScrollCallback> scrollCallbacks;
	std::vector<ScrollInput> scrollInputs;

private:
	friend struct InputHandler;
};