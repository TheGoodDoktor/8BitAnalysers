#pragma once

class FEmuBase;
class FMCPSuggestionQueue;

// Draw the pending MCP suggestions review window.
// Should be called once per frame from the main render loop.
void DrawMCPSuggestionsUI(FEmuBase* pEmu, FMCPSuggestionQueue& queue);
