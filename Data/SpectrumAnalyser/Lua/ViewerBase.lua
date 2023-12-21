

ViewerBase = 
{
    graphicsView = nil,
    width = 256,
    height = 192,
}

function ViewerBase:new(t)
    t = t or {}
    setmetatable(t,self)
    self.__index = self
    return t
end

ZXViewerBase = ViewerBase:new()

function ZXViewerBase:new(t)
    t = t or {}
    setmetatable(t,self)
    self.__index = self
    return t
end

function ZXViewerBase:onAdd()
    self.graphicsView = CreateZXGraphicsView(self.width,self.height)

    if self.Init ~= nil then
        self:Init()
    end
end

function ZXViewerBase:drawOverlayRect(x,y,width,height)
    local scale = self.imageScale
    local xp = self.viewScreenPos.x + (x * scale)
    local yp = self.viewScreenPos.y + (y * scale)
    imgui.DrawList_AddRect(xp,yp,xp + (width * scale),yp + (height * scale), 0xffffffff)
end

function ZXViewerBase:drawOverlayText(x,y,text)
    local scale = self.imageScale
    local xp = self.viewScreenPos.x + (x * scale)
    local yp = self.viewScreenPos.y + (y * scale)
    imgui.DrawList_AddText(xp, yp, 0xffffffff, text)
end

function ZXViewerBase:onDrawUI()
    
    self.imageScale = GetImageScale()
    self.viewScreenPos = imgui.GetCursorScreenPos()

	-- Update and draw to screen
	DrawGraphicsView(self.graphicsView)

    if self.DrawUI ~= nil then
        self:DrawUI()
    end
end













