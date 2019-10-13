#pragma once

struct FSpeccyConfig
{

};

struct FSpeccy
{

};

FSpeccy* InitSpeccy(const FSpeccyConfig& config);
void TickSpeccy(FSpeccy &speccyInstance);
void ShutdownSpeccy(FSpeccy*&pSpeccy);

