#pragma once


#include "Util/GraphicsView.h"


class FBBCGraphicsView : public FGraphicsView
{
public:
	FBBCGraphicsView(int width, int height) :FGraphicsView(width, height) {}

private:
	static const uint32_t Palette[16];

};