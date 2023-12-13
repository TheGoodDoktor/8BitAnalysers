-- This is a template viewer script
-- Use it as a starting point

function DrawScreenToView(graphicsView, screenNo, x, y)

end


ScreenViewer = 
{
    name = "Screen Viewer",
   	screenNo = 0,
	
	onAdd = function(self)
		self.graphicsView = CreateZXGraphicsView(256,256)
        ClearGraphicsView(self.graphicsView, 0)
	end,

    onValueChanged = function(newValue)
        -- TODO: code to update viewer
    end,

	onDrawUI = function(self)
		ClearGraphicsView(self.graphicsView, 0)

		--DrawBlockToView(self.graphicsView, self.blockNo, 0, 0)
		DrawScreenToView(self.graphicsView,self.screenNo, 0, 0)

		-- Update and draw to screen
		DrawGraphicsView(self.graphicsView)
	end,

}

-- Initialise the JN2 viewer
print("JN2 Viewer Initialised")
InitViewers();
AddViewer(ScreenViewer);