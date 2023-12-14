
Viewers = {}

function InitViewers()
   Viewers = {}
end

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