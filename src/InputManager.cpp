#include "InputManager.h"
#include <algorithm>
#include <iostream>

// Define the static variables to fix linker errors
std::vector<InputManager::Keycode> InputManager::keysDown;
std::vector<InputManager::Keycode> InputManager::keysPressed;
std::vector<InputManager::Keycode> InputManager::keysReleased;
std::vector<InputManager::Keycode> InputManager::keysDownBuffer;
std::vector<InputManager::Keycode> InputManager::keysReleasedBuffer;

std::vector<InputManager::Mousecode> InputManager::mouseButtonsDown;
std::vector<InputManager::Mousecode> InputManager::mouseButtonsPressed;
std::vector<InputManager::Mousecode> InputManager::mouseButtonsReleased;
std::vector<InputManager::Mousecode> InputManager::mouseButtonsDownBuffer;
std::vector<InputManager::Mousecode> InputManager::mouseButtonsReleasedBuffer;

Vector2 InputManager::mousePosition;

bool InputManager::GetKeyDown(InputManager::Keycode keycode)
{
	return IsKeyInVector(keysDown, keycode);
}

bool InputManager::GetKey(InputManager::Keycode keycode)
{
	return IsKeyInVector(keysPressed, keycode);
}

bool InputManager::GetKeyUp(InputManager::Keycode keycode)
{
	return IsKeyInVector(keysReleased, keycode);
}

bool InputManager::GetMouseButtonDown(InputManager::Mousecode mousecode)
{
	return IsKeyInVector(mouseButtonsDown, mousecode);
}

bool InputManager::GetMouseButton(InputManager::Mousecode mousecode)
{
	return IsKeyInVector(mouseButtonsPressed, mousecode);
}

bool InputManager::GetMouseButtonUp(InputManager::Mousecode mousecode)
{
	return IsKeyInVector(mouseButtonsReleased, mousecode);
}

template<typename T>
bool InputManager::IsKeyInVector(std::vector<T> vector, T keycode)
{
	std::vector<T>::iterator it = std::find(vector.begin(), vector.end(), keycode);

	if (it == vector.end())
		return false;
	else
		return true;
}

void InputManager::TriggerKeyDownEvent(InputManager::Keycode keycode)
{
	std::vector<InputManager::Keycode>::iterator keyReleasedPos;
	keyReleasedPos = std::find(keysReleasedBuffer.begin(), keysReleasedBuffer.end(), keycode);
	if (keyReleasedPos != keysReleasedBuffer.end())
		keysReleasedBuffer.erase(keyReleasedPos);
	keysDownBuffer.push_back(keycode);
}

void InputManager::TriggerKeyReleaseEvent(InputManager::Keycode keycode)
{
	std::vector<InputManager::Keycode>::iterator it;
	it = std::find(keysDownBuffer.begin(), keysDownBuffer.end(), keycode);
	if (it != keysDownBuffer.end())
		keysDownBuffer.erase(it);
	it = std::find(keysDown.begin(), keysDown.end(), keycode);
	if (it != keysDown.end())
		keysDown.erase(it);
	it = std::find(keysPressed.begin(), keysPressed.end(), keycode);
	if (it != keysPressed.end())
		keysPressed.erase(it);
	keysReleasedBuffer.push_back(keycode);
}

void InputManager::TriggerMouseButtonDownEvent(InputManager::Mousecode mousecode)
{
	std::vector<InputManager::Mousecode>::iterator mouseButtonReleasedPos;
	mouseButtonReleasedPos = std::find(mouseButtonsReleasedBuffer.begin(), mouseButtonsReleasedBuffer.end(), mousecode);
	if (mouseButtonReleasedPos != mouseButtonsReleasedBuffer.end())
		mouseButtonsReleasedBuffer.erase(mouseButtonReleasedPos);
	mouseButtonsDownBuffer.push_back(mousecode);
}

void InputManager::TriggerMouseButtonReleaseEvent(InputManager::Mousecode mousecode)
{
	std::vector<InputManager::Mousecode>::iterator it;
	it = std::find(mouseButtonsDownBuffer.begin(), mouseButtonsDownBuffer.end(), mousecode);
	if (it != mouseButtonsDownBuffer.end())
		mouseButtonsDownBuffer.erase(it);
	it = std::find(mouseButtonsDown.begin(), mouseButtonsDown.end(), mousecode);
	if (it != mouseButtonsDown.end())
		mouseButtonsDown.erase(it);
	it = std::find(mouseButtonsPressed.begin(), mouseButtonsPressed.end(), mousecode);
	if (it != mouseButtonsPressed.end())
		mouseButtonsPressed.erase(it);
	mouseButtonsReleasedBuffer.push_back(mousecode);
}

void InputManager::HandleInputs()
{
	// Handle keyboard actions
	for each (InputManager::Keycode prevDownKey in keysDown)
		keysPressed.push_back(prevDownKey);
	keysDown.clear();
	for each (InputManager::Keycode pressedKey in keysDownBuffer)
		keysDown.push_back(pressedKey);
	keysReleased.clear();
	for each (InputManager::Keycode releasedKey in keysReleasedBuffer)
		keysReleased.push_back(releasedKey);
	keysDownBuffer.clear();
	keysReleasedBuffer.clear();

	// Handle mouse actions
	for each (InputManager::Mousecode prevDownKey in mouseButtonsDown)
		mouseButtonsPressed.push_back(prevDownKey);
	mouseButtonsDown.clear();
	for each (InputManager::Mousecode pressedKey in mouseButtonsDownBuffer)
		mouseButtonsDown.push_back(pressedKey);
	mouseButtonsReleased.clear();
	for each (InputManager::Mousecode releasedKey in mouseButtonsReleasedBuffer)
		mouseButtonsReleased.push_back(releasedKey);
	mouseButtonsDownBuffer.clear();
	mouseButtonsReleasedBuffer.clear();
}
