#pragma once

// interface class
class IInputEventHandler
{
public:
	// keyboard handling
	virtual void	OnKeyUp(int keyCode) = 0;
	virtual void	OnKeyDown(int keyCode) = 0;
	virtual void	OnChar(int charCode) = 0;

	// gamepad handling
	virtual void	OnGamepadUpdated(int mask) = 0;
};

// this will be implemented in platform specific files
void SetInputEventHandler(IInputEventHandler* pHandler);