#pragma once

class MouseInput
{
public:
	static MouseInput* GetMouseInput();

	enum MouseKey {
		None, Left, Right, Middle
	};

	float x;
	float y;
	MouseKey pressed;
	//MouseKey released;

private:
	static MouseInput* input;
	MouseInput() : x(0), y(0), pressed(None) {};
};
