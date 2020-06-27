#pragma once

#include <vector>
#include "vector.h"

class InputManager
{
public:
	enum class Keycode
	{
		Spacebar,
		Alt,
		Strg
	};

	enum class Mousecode
	{
		Left, 
		Right, 
		Middle
	};

private:
	// Variables for keyboard inputs 
	static std::vector<InputManager::Keycode> keysDownBuffer;
	static std::vector<InputManager::Keycode> keysReleasedBuffer;
	static std::vector<InputManager::Keycode> keysDown;
	static std::vector<InputManager::Keycode> keysPressed;
	static std::vector<InputManager::Keycode> keysReleased;

	// Variables for mouse inputs
	static std::vector<InputManager::Mousecode> mouseButtonsDownBuffer;
	static std::vector<InputManager::Mousecode> mouseButtonsReleasedBuffer;
	static std::vector<InputManager::Mousecode> mouseButtonsDown;
	static std::vector<InputManager::Mousecode> mouseButtonsPressed;
	static std::vector<InputManager::Mousecode> mouseButtonsReleased;
	
protected:
	template<typename T>
	static bool IsKeyInVector(std::vector<T> vector, T keycode);
	static void TriggerKeyDownEvent(InputManager::Keycode keycode);
	static void TriggerKeyReleaseEvent(InputManager::Keycode keycode);
	static void TriggerMouseButtonDownEvent(InputManager::Mousecode mousecode);
	static void TriggerMouseButtonReleaseEvent(InputManager::Mousecode mousecode);
	static void HandleInputs();

public:
	static bool GetKeyDown(InputManager::Keycode keycode);
	static bool GetKey(InputManager::Keycode keycode);
	static bool GetKeyUp(InputManager::Keycode keycode);
	static bool GetMouseButtonDown(InputManager::Mousecode mousecode);
	static bool GetMouseButton(InputManager::Mousecode mousecode);
	static bool GetMouseButtonUp(InputManager::Mousecode mousecode);
	static Vector2 mousePosition;
};