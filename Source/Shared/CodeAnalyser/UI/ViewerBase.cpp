#include "ViewerBase.h"

#include "optick.h"

// sam. added this base class draw call so we can profile all viewers using optick
void FViewerBase::DrawUIBase()
{
	OPTICK_EVENT_DYNAMIC(GetName());

	DrawUI();
}