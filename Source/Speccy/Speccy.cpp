#include "Speccy.h"

FSpeccy* InitSpeccy(const FSpeccyConfig& config)
{
	FSpeccy *pNewInstance = new FSpeccy();

	return pNewInstance;
}

void TickSpeccy(FSpeccy &speccyInstance)
{

}

void ShutdownSpeccy(FSpeccy*&pSpeccy)
{
	delete pSpeccy;
	pSpeccy = nullptr;
}
