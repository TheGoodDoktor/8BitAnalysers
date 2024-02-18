
print("ViewerBase start")

ViewerBase = 
{
    graphicsView = nil,
    width = 320,
    height = 200,
}

function ViewerBase:new(t)
    t = t or {}
    setmetatable(t,self)
    self.__index = self
    return t
end

CPCViewerBase = ViewerBase:new()

function CPCViewerBase:new(t)
    t = t or {}
    setmetatable(t,self)
    self.__index = self
    return t
end

function CPCViewerBase:onAdd()
    self.graphicsView = CreateCPCGraphicsView(self.width,self.height)

    if self.Init ~= nil then
        self:Init()
    end
end

function CPCViewerBase:drawOverlayRect(x,y,width,height)
    local scale = self.imageScale
    local xp = self.viewScreenPos.x + (x * scale)
    local yp = self.viewScreenPos.y + (y * scale)
    imgui.DrawList_AddRect(xp,yp,xp + (width * scale),yp + (height * scale), 0xffffffff)
end

function CPCViewerBase:drawOverlayText(x,y,text)
    local scale = self.imageScale
    local xp = self.viewScreenPos.x + (x * scale)
    local yp = self.viewScreenPos.y + (y * scale)
    imgui.DrawList_AddText(xp, yp, 0xffffffff, text)
end

function CPCViewerBase:onDrawUI()
    
    self.imageScale = GetImageScale()
    self.viewScreenPos = imgui.GetCursorScreenPos()

	-- Update and draw to screen
	DrawGraphicsView(self.graphicsView)

    if self.DrawUI ~= nil then
        self:DrawUI()
    end
end
print("ViewerBase end")
