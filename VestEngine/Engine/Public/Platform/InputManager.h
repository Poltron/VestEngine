#pragma once

#include <functional>
#include <unordered_map>

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

enum class EInputMode
{
	Capture,
	Free
};

class InputManager
{
public:
	virtual void initialize() = 0;
	void processInput(double inDeltaTime);

protected:
	virtual void pollEvents() = 0;

public:
	using KeyCallback = std::function<void(int/* state */, int/* mods */, double/* deltaTime */)>;
	using MouseCallback = std::function<void(int /* state */, double/* deltaTime */)>;
	using CursorPosCallback = std::function<void(double/* xPos */, double/* yPos */, double/* deltaTime */)>;
	using ScrollCallback = std::function<void(double/* x */, double/* y */, double/* deltaTime */)>;

	void registerKeyCallback(int key, KeyCallback inCallback);
	void registerMouseCallback(int button, MouseCallback inCallback);
	void registerCursorPosCallback(CursorPosCallback inCallback);
	void registerScrollCallback(ScrollCallback inScrollback);

protected:
	std::unordered_map<int, std::vector<KeyCallback>> keyCallbacks;
	std::unordered_map<int, std::vector<MouseCallback>> mouseCallbacks;
	std::vector<CursorPosCallback> cursorPosCallbacks;
	std::vector<ScrollCallback> scrollCallbacks;

	// todo : use StackAllocators
	std::vector<KeyInput> keyInputs;
	std::vector<MouseInput> mouseInputs;
	std::vector<CursorPosInput> cursorPosInputs;
	std::vector<ScrollInput> scrollInputs;
};