#include "CPCLuaAPI.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "Util/GraphicsView.h"

static int CreateCPCGraphicsView(lua_State *pState)
{
	const int width = (int)luaL_optnumber(pState, 1, 320);
	const int height = (int)luaL_optnumber(pState, 2, 256);
 
	void *mem = lua_newuserdata(pState, sizeof(FGraphicsView));
	luaL_setmetatable(pState, "GraphicsViewMT");

	// create object using placement new with the memory we allocated for the user data
	FGraphicsView* pGraphicsView = new(mem) FGraphicsView(width,height);
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

static int DrawCPCMode0Image(lua_State *pState)
{
	/*
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
		const int stride = (int)luaL_optinteger(pState, 8, 1);
		if (xp < pGraphicsView->GetWidth() && yp < pGraphicsView->GetHeight())
			pGraphicsView->DrawBitImage(pImageData, xp, yp, widthChars, heightChars, attrib, stride);
	}
	*/
    return 0;
}

static int DrawCPCMode1Image(lua_State* pState)
{
	FGraphicsView* pGraphicsView = (FGraphicsView*)lua_touserdata(pState, 1 );
	if (pGraphicsView == nullptr)
		return 0;

	const uint8_t* pImageData = (const uint8_t*)lua_touserdata(pState,2);
	if (pImageData == nullptr)
		return 0;

	const int xp = (int)luaL_optinteger(pState,3, 0);
	const int yp = (int)luaL_optinteger(pState,4, 0);
	const int widthPixels = (int)luaL_optinteger(pState,5,1);
	const int heightPixels = (int)luaL_optinteger(pState,6,1);
	if (lua_islightuserdata(pState, 7))
	{
		const uint8_t* pColData = (const uint8_t*)lua_touserdata(pState, 7);
		if (pColData == nullptr)
			return 0;

		//const uint32_t palette[4] = { 0xffffffff, 0xffff0000, 0xff00ff00, 0xff0000ff };
		pGraphicsView->Draw2BppImageAt(pImageData, xp, yp, widthPixels, heightPixels, palette);
	}
	/*
	else
	{
		const uint8_t attrib = (uint8_t)luaL_optinteger(pState,7,0x47);
		const int stride = (int)luaL_optinteger(pState, 8, 1);
		if (xp < pGraphicsView->GetWidth() && yp < pGraphicsView->GetHeight())
			pGraphicsView->DrawBitImage(pImageData, xp, yp, widthChars, heightChars, attrib, stride);
	}
	*/
	
	return 0;
}

static const luaL_Reg cpclib[] =
{
    {"CreateCPCGraphicsView", CreateCPCGraphicsView},
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
