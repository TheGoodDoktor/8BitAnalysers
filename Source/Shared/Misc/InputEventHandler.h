#pragma once

// interface class
class IInputEventHandler
{
public:
	virtual void	OnKeyUp(int keyCode) = 0;
	virtual void	OnKeyDown(int keyCode) = 0;
	virtual void	OnChar(int charCode) = 0;
};