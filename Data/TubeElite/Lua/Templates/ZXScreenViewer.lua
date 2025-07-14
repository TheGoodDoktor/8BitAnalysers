-- This is a template viewer script
-- Use it as a starting point

function DrawScreenToView(graphicsView, screenNo, x, y)

    -- TODO: draw screen pixels to graphicsView
end

ScreenViewer = 
{
    name = "Screen Viewer",
   	screenNo = 0,
    screenMin = 0,
    screenMax = 100,
	
	onAdd = function(self)
		self.graphicsView = CreateZXGraphicsView(256,256)
        ClearGraphicsView(self.graphicsView, 0)
	end,

	onDrawUI = function(self)
		local changed = false

        -- Use ImGui widget for setting screen number to draw
		changed, self.screenNo = imgui.InputInt("screen number",self.screenNo)

		if changed == true then
			ClearGraphicsView(self.graphicsView, 0)
			DrawScreenToView(self.graphicsView,self.screenNo, 0, 0)
		end

		-- Update and draw to screen
		DrawGraphicsView(self.graphicsView)
	end,

}

-- Initialise the template viewer
print("Template Viewer Initialised")
AddViewer(ScreenViewer);