
-- enumerate types

EDataType =
{
   Byte = 0,
   ByteArray = 1,
   Word = 2,
   WordArray = 3,
   Text = 4,		
   Bitmap = 5,
   CharacterMap = 6,
   ScreenPixels = 7,	
   Image = 8,		
   Blob = 9,		
   ColAttr = 10,	
   InstructionOperand = 11,	
   Struct = 12,		
   Flags = 13,		
}

EDataItemDisplayType = 
{
   Unknown = 0,
   Pointer = 1,
   JumpAddress = 2,
   Decimal = 3,
   Hex = 4,
   Binary = 5,
   Bitmap = 6,			-- 1 bit per pixel
   ColMap2Bpp_CPC = 7,	-- Amstrad CPC mode 1 format
   ColMap4Bpp_CPC = 8,	-- Amstrad CPC mode 0 format
   ColMapMulticolour_C64 = 9,	-- C64 Multicolour format
   UnsignedNumber = 10,
   SignedNumber = 11,
   Ascii = 12
}

function SetDataItemTypeAndComment(address,displayType,comment)
	SetDataItemComment(address,comment)		
	SetDataItemDisplayType(address, displayType)
end

-- viewer related
Viewers = {}

function AddViewer(viewer)
    print(viewer.name .. " added")
    table.insert(Viewers,viewer)
    if(viewer.onAdd ~= nil) then
        --print("onAdd called")
        viewer:onAdd()
    end
end

-- util functions
function dumpFunc(o)
--[[
	local key,value
    if type(o) == 'table' then
       local s = '{ '
       for key,value in pairs(o) do
          if type(key) ~= 'number' then key = '"'..key..'"' end
          s = s .. '['..key..'] = ' .. dumpFunc(value) .. ','
       end
       return s .. '} '
    else
       return tostring(o)
    end
]]
	return "not implemented"
 end

 function dump(o)
    print(dumpFunc(o))
 end

-- Off screen buffers
OffScreenBufferHandlers = {}

function RegisterOffscreenBufferHandler(handler)
   print("Offscreen buffer handler " .. handler.name .. " added")
   --table.insert(OffScreenBufferHandlers, handler)
   OffScreenBufferHandlers[handler.name] = handler
end

-- CPU access tables
Z80 = {}
Z80Meta = {}

Z80Meta.__index = function(table,key)
	return GetRegValue(key)
end
setmetatable(Z80,Z80Meta)
