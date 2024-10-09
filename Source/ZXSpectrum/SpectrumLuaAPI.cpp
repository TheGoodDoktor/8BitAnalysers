#include "SpectrumLuaAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "Viewers/ZXGraphicsView.h"
#include "LuaScripting/LuaDocs.h"

static int CreateZXGraphicsView(lua_State *pState)
{
    const int width = (int)luaL_optnumber(pState, 1, 256);
    const int height = (int)luaL_optnumber(pState, 2, 256);
    
	void *mem = lua_newuserdata(pState, sizeof(FZXGraphicsView));
	luaL_setmetatable(pState, "GraphicsViewMT");

	// create object using placement new with the memory we allocated for the user data
    FZXGraphicsView* pGraphicsView = new(mem) FZXGraphicsView(width,height);
    pGraphicsView->Clear();
    //lua_pushlightuserdata(pState, pGraphicsView);
    
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

static int DrawZXBitImage(lua_State *pState)
{
    FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1 );
	if (pGraphicsView == nullptr)
		return 0;

    const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState,2);
	if (pImageData == nullptr)
		return 0;

    const int xp = (int)luaL_optinteger(pState,3, 0);
    const int yp = (int)luaL_optinteger(pState,4, 0);
    const int widthChars = (int)luaL_optinteger(pState,5,1);
    const int heightChars = (int)luaL_optinteger(pState,6,1);
	if (lua_islightuserdata(pState, 7))
	{
		const uint8_t* pAttribData = (const uint8_t*)lua_touserdata(pState, 7);
		if (pAttribData == nullptr)
			return 0;

		pGraphicsView->DrawBitImageWithAttribs(pImageData, xp, yp, widthChars, heightChars, pAttribData);
	}
	else
    {
		const uint8_t attrib = (uint8_t)luaL_optinteger(pState,7,0x47);
		const int stride = (int)luaL_optinteger(pState, 8, widthChars);
		const bool bMask = luaL_optinteger(pState, 9, 0);
		pGraphicsView->DrawBitImage(pImageData, xp, yp, widthChars, heightChars, attrib, stride, bMask);
	}

    return 0;
}

static int DrawZXBitImageMask(lua_State* pState)
{
	FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1);
	if (pGraphicsView == nullptr)
		return 0;

	const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState, 2);
	if (pImageData == nullptr)
		return 0;

	const int xp = (int)luaL_optinteger(pState, 3, 0);
	const int yp = (int)luaL_optinteger(pState, 4, 0);
	const int widthChars = (int)luaL_optinteger(pState, 5, 1);
	const int heightChars = (int)luaL_optinteger(pState, 6, 1);
	const uint8_t attrib = (uint8_t)luaL_optinteger(pState, 7, 0x47);
	const int stride = (int)luaL_optinteger(pState, 8, 1);
	pGraphicsView->DrawBitImageMask(pImageData, xp, yp, widthChars, heightChars, attrib, stride);
	
	return 0;
}

static int DrawZXBitImageFineY(lua_State* pState)
{
	FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1);
	if (pGraphicsView == nullptr)
		return 0;

	const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState, 2);
	if (pImageData == nullptr)
		return 0;

	const int xp = (int)luaL_optinteger(pState, 3, 0);
	const int yp = (int)luaL_optinteger(pState, 4, 0);
	const int widthChars = (int)luaL_optinteger(pState, 5, 1);
	const int heightPixels = (int)luaL_optinteger(pState, 6, 1);
	const uint8_t attrib = (uint8_t)luaL_optinteger(pState, 7, 0x47);
	const int stride = (int)luaL_optinteger(pState, 8, 1);

	pGraphicsView->DrawBitImageFineY(pImageData, xp, yp, widthChars, heightPixels, attrib, stride);
	return 0;
}

static const luaL_Reg spectrumlib[] =
{
    {"CreateZXGraphicsView", CreateZXGraphicsView},
    {"DrawZXBitImage", DrawZXBitImage},
	{"DrawZXBitImageMask", DrawZXBitImageMask},
	{"DrawZXBitImageFineY", DrawZXBitImageFineY},
    //{"readbyte", readbyte},
    //{"readword", readword},
    //{"addViewer", addViewer},
    {NULL, NULL}    // terminator
};

// meta table for graphics view
static const luaL_Reg graphicsViewMT[] = 
{
	{"__gc",	FreeGraphicsView},

	{NULL, NULL}    // terminator
};

void AddZXLibLuaDocs(void)
{
	FLuaDocLib& zxLuaDocLib = AddLuaDocLib("ZX Spectrum API");
	LoadLuaDocLibFromJson(zxLuaDocLib, "Lua\\Docs\\SpectrumLuaAPIDocs.json");
	zxLuaDocLib.Verify(spectrumlib);
}

int RegisterSpectrumLuaAPI(lua_State *pState)
{
	lua_getglobal(pState, "_G");
	luaL_setfuncs(pState, spectrumlib, 0);  // for Lua versions 5.2 or greater
	lua_pop(pState, 1);

	luaL_newmetatable(pState,"GraphicsViewMT");
	luaL_setfuncs(pState, graphicsViewMT, 0);
	lua_pop(pState, 1);

	return 1;
}
