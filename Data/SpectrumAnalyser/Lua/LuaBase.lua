
Viewers = {}

function AddViewer(viewer)
    table.insert(Viewers,viewer)
    if(viewer.onAdd ~= nil) then
        viewer:onAdd()
    end
end