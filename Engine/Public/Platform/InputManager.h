#pragma once

#include <functional>
#include <unordered_map>

#include "Platform/Input.h"

struct KeyInput
{
	input::EKey key;
	input::EInputState state;
	input::EKeyModifier mods;

	KeyInput(input::EKey inKey, input::EInputState inState, input::EKeyModifier inMods)
		: key(inKey), state(inState), mods(inMods)
	{}
};

struct MouseInput
{
	input::EMouseButton button;
	input::EInputState state;

	MouseInput(input::EMouseButton inButton, input::EInputState inState)
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
	virtual bool initialize();
	virtual void shutdown();
	void processInput(double inDeltaTime);

	virtual input::ECursorInputMode getCursorInputMode() = 0;
	virtual void setCursorInputMode(input::ECursorInputMode inButton) = 0;
	virtual bool isKeyPressed(input::EKey inKey) = 0;
	virtual bool isButtonPressed(input::EMouseButton inButton) = 0;

protected:
	virtual void pollEvents() = 0;

public:
	using KeyCallback = std::function<void(input::EInputState/* state */, input::EKeyModifier/* mods */, double/* deltaTime */)>;
	using MouseCallback = std::function<void(input::EInputState /* state */, double/* deltaTime */)>;
	using CursorPosCallback = std::function<void(double/* xPos */, double/* yPos */, double/* deltaTime */)>;
	using ScrollCallback = std::function<void(double/* x */, double/* y */, double/* deltaTime */)>;

	input::InputCallbackHandle registerKeyCallback(input::EKey inKey, KeyCallback inCallback);
	input::InputCallbackHandle registerMouseCallback(input::EMouseButton inButton, MouseCallback inCallback);
	input::InputCallbackHandle registerCursorPosCallback(CursorPosCallback inCallback);
	input::InputCallbackHandle registerScrollCallback(ScrollCallback inScrollback);

	void unregisterKeyCallback(input::EKey inKey, input::InputCallbackHandle inHandle);
	void unregisterMouseCallback(input::EMouseButton inButton, input::InputCallbackHandle inHandle);
	void unregisterCursorCallback(input::InputCallbackHandle inHandle);
	void unregisterScrollCallback(input::InputCallbackHandle inHandle);

protected:
	std::unordered_map<input::EKey, std::vector<KeyCallback>> keyCallbacks;
	std::unordered_map<input::EMouseButton, std::vector<MouseCallback>> mouseCallbacks;
	std::vector<CursorPosCallback> cursorPosCallbacks;
	std::vector<ScrollCallback> scrollCallbacks;

	// todo : use StackAllocators
	std::vector<KeyInput> keyInputs;
	std::vector<MouseInput> mouseInputs;
	std::vector<CursorPosInput> cursorPosInputs;
	std::vector<ScrollInput> scrollInputs;
};