-- This contains base CPC routines used by all CPC games

print("CPCBase Loaded")


ScreenOverlay = 
{ 
    viewScreenPos = { x=0, y=0 }, 
    imageScale = 1,
    drawCol = 0xffffffff 
}

function ScreenOverlay:setDrawCol(col)
    self.drawCol = col
end

function ScreenOverlay:drawRect(x,y,width,height)
    local scale = self.imageScale
    local xp = self.viewScreenPos.x + (x * scale)
    local yp = self.viewScreenPos.y + (y * scale)
    imgui.DrawList_AddRect(xp, yp, xp + (width * scale),yp + (height * scale), self.drawCol)
end

function ScreenOverlay:drawLine(x1,y1,x2,y2)
    local scale = self.imageScale
    local xp1 = self.viewScreenPos.x + (x1 * scale)
    local yp1 = self.viewScreenPos.y + (y1 * scale)
    local xp2 = self.viewScreenPos.x + (x2 * scale)
    local yp2 = self.viewScreenPos.y + (y2 * scale)
    imgui.DrawList_AddLine(xp1, yp1, xp2, yp2, self.drawCol)
end

function ScreenOverlay:drawCoord(x,y)
	self:drawLine(x,0,x,199)
	self:drawLine(0,y,319,y)
end

function ScreenOverlay:drawText(x,y,text)
    local scale = self.imageScale
    local xp = self.viewScreenPos.x + (x * scale)
    local yp = self.viewScreenPos.y + (y * scale)
    imgui.DrawList_AddText(xp, yp, self.drawCol, text)
end


function OnScreenDraw(x,y,scale)

    ScreenOverlay.viewScreenPos.x = x + ((320 - 320) / 2) * scale
    ScreenOverlay.viewScreenPos.y = y + ((256 - 200) / 2) * scale
    ScreenOverlay.imageScale = scale

    if ScreenOverlay.drawFunc ~= nil then
        ScreenOverlay:drawFunc()
    end
end

function SetScreenDrawOverlayFunc(overlayFunc)
    ScreenOverlay.drawFunc = overlayFunc
end
