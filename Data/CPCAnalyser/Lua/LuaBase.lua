
print("luabase start")

-- enumerate types

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
   SignedNumber = 11
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
    if type(o) == 'table' then
       local s = '{ '
       for k,v in pairs(o) do
          if type(k) ~= 'number' then k = '"'..k..'"' end
          s = s .. '['..k..'] = ' .. dumpFunc(v) .. ','
       end
       return s .. '} '
    else
       return tostring(o)
    end
 end

 function dump(o)
    print(dumpFunc(o))
 end