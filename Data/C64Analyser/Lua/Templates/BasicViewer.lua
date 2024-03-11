-- Basic viewer template

-- rename this
BasicViewer = 
{
    name = "Basic Viewer",  -- rename

    -- add your own viewer members here
    
    onAdd = function(self)
        -- gets called when view is added
    end,

    onDrawUI = function(self)
        -- gets called every frame
        imgui.Text("This viewer is called: " .. tostring(self.name))

    end
}

-- add viewer when file gets loaded
AddViewer(BasicViewer);