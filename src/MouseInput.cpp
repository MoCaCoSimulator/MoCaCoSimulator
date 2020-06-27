#pragma once
#include "MouseInput.h"

MouseInput* MouseInput::input;

MouseInput* MouseInput::GetMouseInput()
{
	if (!input)
		input = new MouseInput();
	return input;
}
