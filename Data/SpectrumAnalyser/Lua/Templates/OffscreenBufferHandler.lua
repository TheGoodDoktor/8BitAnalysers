local OffscreenBufferHandler = {}

function OffscreenBufferHandler:Draw(graphicsView, address, xSize, ySize)
    -- Draw logic here
    for y = 0, ySize - 1 do
        for x = 0, xSize - 1, 8 do
            local charLine = ReadByte(address)
            DrawPixelLineHeatmap(graphicsView, address, x, y)
            --state:AdvanceAddressRef(address, 1)
            address = address + 1
        end
    end

end

function OffscreenBufferHandler:GetAddressOffsetFromPos(x, y, xSize, ySize)
    -- Calculate address offset from position (x, y) here
    local xSizeChars = xSize >> 3;

	return (x / 8) + (y * xSizeChars);
end
