#include "CPCLuaAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "Viewers/CPCGraphicsView.h"

static int CreateCPCGraphicsView(lua_State *pState)
{
	const int width = (int)luaL_optnumber(pState, 1, 320);
	const int height = (int)luaL_optnumber(pState, 2, 256);
 
	void *mem = lua_newuserdata(pState, sizeof(FGraphicsView));
	luaL_setmetatable(pState, "GraphicsViewMT");

	// create object using placement new with the memory we allocated for the user data
	FCPCGraphicsView* pGraphicsView = new(mem) FCPCGraphicsView(width,height);
	pGraphicsView->Clear();
	
	return 1;
}


static int FreeGraphicsView(lua_State* pState)
{
	FGraphicsView* pGraphicsView = (FGraphicsView*)lua_touserdata(pState, 1);
	if (pGraphicsView == nullptr)
		return 0;

	pGraphicsView->~FGraphicsView();	// call destructor
	return 0;
}

static int DrawCPCImage(lua_State* pState, int mode)
{
	FCPCGraphicsView* pGraphicsView = (FCPCGraphicsView*)lua_touserdata(pState, 1);
	if (pGraphicsView == nullptr)
		return 0;

	const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState, 2);
	if (pImageData == nullptr)
		return 0;

	const int xp = (int)luaL_optinteger(pState, 3, 0);
	const int yp = (int)luaL_optinteger(pState, 4, 0);
	const int widthPixels = (int)luaL_optinteger(pState, 5, 8);
	const int heightPixels = (int)luaL_optinteger(pState, 6, 8);
	const int paletteIndex = (int)luaL_optinteger(pState, 7, 0);

	if (mode == 0)
		pGraphicsView->DrawMode0Image(pImageData, xp, yp, widthPixels, heightPixels, paletteIndex);
	else
		pGraphicsView->DrawMode1Image(pImageData, xp, yp, widthPixels, heightPixels, paletteIndex);

	return 0;
}

static int DrawCPCBitImage(lua_State* pState)
{
	FCPCGraphicsView* pGraphicsView = (FCPCGraphicsView*)lua_touserdata(pState, 1);
	if (pGraphicsView == nullptr)
		return 0;

	const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState, 2);
	if (pImageData == nullptr)
		return 0;

	const int xp = (int)luaL_optinteger(pState, 3, 0);
	const int yp = (int)luaL_optinteger(pState, 4, 0);
	const int widthPixels = (int)luaL_optinteger(pState, 5, 8);
	const int heightPixels = (int)luaL_optinteger(pState, 6, 8);
	const int paletteIndex = (int)luaL_optinteger(pState, 7, 0);
	const int paperColIndex = (int)luaL_optinteger(pState, 8, 0);
	const int inkColIndex = (int)luaL_optinteger(pState, 9, 1);

	uint32_t cols[] = { 0, 0xffffffff };
	
	if (const FPaletteEntry* pPaletteEntry = GetPaletteEntry(paletteIndex))
	{
		const uint32_t* pPalette = GetPaletteFromPaletteNo(paletteIndex);
		if (pPalette)
		{
			if (paperColIndex < pPaletteEntry->NoColours)
			{
				cols[0] = pPalette[paperColIndex];
			}
			if (inkColIndex < pPaletteEntry->NoColours)
			{
				cols[1] = pPalette[inkColIndex];
			}
		}
	}

	pGraphicsView->Draw1BppImageAt(pImageData, xp, yp, widthPixels, heightPixels, cols);

	return 0;
}

static int DrawCPCMode0Image(lua_State *pState)
{
	return DrawCPCImage(pState, 0);
}

static int DrawCPCMode1Image(lua_State* pState)
{
	return DrawCPCImage(pState, 1);
}

static const luaL_Reg cpclib[] =
{
	{"CreateCPCGraphicsView", CreateCPCGraphicsView},
	{"DrawCPCBitImage", DrawCPCBitImage},
	{"DrawCPCMode0Image", DrawCPCMode0Image},
	{"DrawCPCMode1Image", DrawCPCMode1Image},

	{NULL, NULL}    // terminator
};

// meta table for graphics view
static const luaL_Reg graphicsViewMT[] = 
{
	{"__gc",	FreeGraphicsView},

	{NULL, NULL}    // terminator
};

int RegisterCPCLuaAPI(lua_State *pState)
{
	lua_getglobal(pState, "_G");
	luaL_setfuncs(pState, cpclib, 0);  // for Lua versions 5.2 or greater
	lua_pop(pState, 1);

	luaL_newmetatable(pState,"GraphicsViewMT");
	luaL_setfuncs(pState, graphicsViewMT, 0);
	lua_pop(pState, 1);

	return 1;
}
