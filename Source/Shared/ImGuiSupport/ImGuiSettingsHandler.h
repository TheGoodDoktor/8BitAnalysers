#pragma once

class FEmuBase;

// Registers a custom handler for adding our own data to imgui.ini
void RegisterImguiSettingsHandler(FEmuBase* pEmu);
