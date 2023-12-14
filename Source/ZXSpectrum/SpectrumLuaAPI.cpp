#include "SpectrumLuaAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "Viewers/ZXGraphicsView.h"

static int CreateZXGraphicsView(lua_State *pState)
{
    const int width = (int)luaL_optnumber(pState, 1, 256);
    const int height = (int)luaL_optnumber(pState, 2, 256);
    
    FZXGraphicsView* pGraphicsView = new FZXGraphicsView(width,height);
    pGraphicsView->Clear();
    lua_pushlightuserdata(pState, pGraphicsView);
    
    return 1;
}

static int ClearGraphicsView(lua_State *pState)
{
    FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1 );
	if(pGraphicsView == nullptr)
		return 0;

    uint32_t clearCol = 0;
    if(lua_isinteger(pState, 2))
        clearCol = lua_tointeger(pState, 2);
    pGraphicsView->Clear(clearCol);
    return 0;
}

static int DrawGraphicsView(lua_State *pState)
{
    FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1 );
	if (pGraphicsView == nullptr)
		return 0;

    pGraphicsView->UpdateTexture();
    pGraphicsView->Draw();
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
		pGraphicsView->DrawBitImage(pImageData, xp, yp, widthChars, heightChars, attrib);
	}

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

	pGraphicsView->DrawBitImageFineY(pImageData, xp, yp, widthChars, heightPixels, attrib);
}

static const luaL_Reg spectrumlib[] =
{
    {"CreateZXGraphicsView", CreateZXGraphicsView},
    {"ClearGraphicsView", ClearGraphicsView},
    {"DrawGraphicsView", DrawGraphicsView},
    {"DrawZXBitImage", DrawZXBitImage},
	{"DrawZXBitImageFineY", DrawZXBitImageFineY},
    //{"readbyte", readbyte},
    //{"readword", readword},
    //{"addViewer", addViewer},
    {NULL, NULL}    // terminator
};


int RegisterSpectrumLuaAPI(lua_State *pState)
{
    lua_getglobal(pState, "_G");
    luaL_setfuncs(pState, spectrumlib, 0);  // for Lua versions 5.2 or greater
    lua_pop(pState, 1);
    return 1;
}
