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
    // TODO: validate?
    const int width = (int)lua_tointeger(pState, 1);
    const int height = (int)lua_tointeger(pState, 2);
    
    FZXGraphicsView* pGraphicsView = new FZXGraphicsView(width,height);
    pGraphicsView->Clear();
    lua_pushlightuserdata(pState, pGraphicsView);
    
    return 1;
}

static int ClearGraphicsView(lua_State *pState)
{
    FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1 );
    uint32_t clearCol = 0;
    if(lua_isinteger(pState, 2))
        clearCol = lua_tointeger(pState, 2);
    pGraphicsView->Clear(clearCol);
    return 0;
}

static int DrawGraphicsView(lua_State *pState)
{
    FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1 );
    pGraphicsView->UpdateTexture();
    pGraphicsView->Draw();
    return 0;
}

static int DrawZXBitImage(lua_State *pState)
{
    // TODO: validate?
    FZXGraphicsView* pGraphicsView = (FZXGraphicsView*)lua_touserdata(pState, 1 );
    const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState,2);
    const int xp = (int)lua_tointeger(pState,3);
    const int yp = (int)lua_tointeger(pState,4);
    const int widthChars = (int)lua_tointeger(pState,5);
    const int heightChars = (int)lua_tointeger(pState,6);
    const uint8_t attrib = (uint8_t)lua_tointeger(pState,7);
    pGraphicsView->DrawBitImage(pImageData, xp, yp, widthChars, heightChars, attrib);
    return 0;
}

static const luaL_Reg spectrumlib[] =
{
    {"CreateZXGraphicsView", CreateZXGraphicsView},
    {"ClearGraphicsView", ClearGraphicsView},
    {"DrawGraphicsView", DrawGraphicsView},
    {"DrawZXBitImage", DrawZXBitImage},
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
