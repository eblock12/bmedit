#include "tga.h"

void TGA::TGAImage::LoadFromStream(FILE *file, long fileSize)
{
    long startPosition = ftell(file);

    fread(&header.idLength, 1, 1, file);
    fread(&header.colorMapType, 1, 1, file);
    fread(&header.imageType, 1, 1, file);

    fread(&header.colorMapInfo.firstEntryIndex, 2, 1, file);
    fread(&header.colorMapInfo.length, 2, 1, file);
    fread(&header.colorMapInfo.entrySize, 1, 1, file);

    fread(&header.imageInfo.xOrigin, 2, 1, file);
    fread(&header.imageInfo.yOrigin, 2, 1, file);
    fread(&header.imageInfo.width, 2, 1, file);
    fread(&header.imageInfo.height, 2, 1, file);
    fread(&header.imageInfo.pixelDepth, 1, 1, file);
    fread(&header.imageInfo.descriptor, 1, 1, file);

    if ((header.imageInfo.pixelDepth != 24) && (header.imageInfo.pixelDepth != 32))
        return;

    if (header.idLength > 0)
    {
        data.id = new unsigned char[header.idLength];
        fread(data.id, 1, header.idLength, file);
    }

    if (header.colorMapType != 0)
    {
        int size = header.colorMapInfo.length * header.colorMapInfo.entrySize;
        data.colorMapData = new unsigned char[size];
        fread(data.colorMapData, 1, size, file);
    }

    long imageSize = fileSize - ftell(file) + startPosition;
    data.imageData = new unsigned char[imageSize];
    fread(data.imageData, 1, imageSize, file);
}

void TGA::TGAImage::DrawToBitmap(Color *bitmap)
{
    int pixelOffset, startX, startY, endX, endY, deltaX, deltaY, plotX, plotY;
    unsigned char r = 0, g = 0, b = 0, a = 255;

	if ((header.imageInfo.descriptor & 0x10) == 0)
	{
		startX = 0;
		endX = header.imageInfo.width;
		deltaX = 1;
	}
	else
	{
		startX = header.imageInfo.width - 1;
		endX = 0;
		deltaX = -1;
	}

	if ((header.imageInfo.descriptor & 0x20) == 0)
	{
		startY = header.imageInfo.height - 1;
		endY = -1;
		deltaY = -1;
	}
	else
	{
		startY = 0;
		endY = header.imageInfo.height;
		deltaY = 1;
	}

	if (header.imageType == 10)
		DrawCompressed(bitmap, startX, startY, endX, endY, deltaX, deltaY);
    else
    {
        plotX = plotY = 0;
	    for (int y = startY; y != endY; y += deltaY)
	    {
		    for (int x = startX; x != endX; x += deltaX)
		    {
			    pixelOffset = ((x + (y * header.imageInfo.width)) * (header.imageInfo.pixelDepth / 8));
			    if (header.imageType == 2)
			    {
				    b = data.imageData[pixelOffset];
				    g = data.imageData[pixelOffset+1];
				    r = data.imageData[pixelOffset+2];
                    if (header.imageInfo.pixelDepth == 32)
                        a = data.imageData[pixelOffset+3];
			    }
                bitmap[plotX + plotY * header.imageInfo.width].r = b * (a / 255);
                bitmap[plotX + plotY * header.imageInfo.width].g = g * (a / 255);
                bitmap[plotX + plotY * header.imageInfo.width].b = r * (a / 255);
                bitmap[plotX + plotY * header.imageInfo.width].a = a;
			    plotX++;
		    }
		    plotX = 0;
		    plotY++;
	    }
    }
}

void TGA::TGAImage::DrawCompressed(Color *bitmap, int startX, int startY, int endX, int endY, int deltaX, int deltaY)
{
	int plotX = 0, plotY = 0, pixelOffset = 0, packetLength = 0;
	unsigned char r = 0, g = 0, b = 0, a = 0, rlePacketInfo = 0;
	bool isPacketRaw = false;

	for (plotY = startY; plotY != endY; plotY += deltaY)
	{
		for (plotX = startX; plotX != endX; plotX += deltaX)
		{
			if (packetLength <= 0)
			{
				rlePacketInfo = data.imageData[pixelOffset];
				packetLength = (rlePacketInfo & 0x7F) + 1;
				pixelOffset++;
				if ((rlePacketInfo & 0x80) == 0x80)
				{
					b = data.imageData[pixelOffset];
					g = data.imageData[pixelOffset+1];
					r = data.imageData[pixelOffset+2];
                    if (header.imageInfo.pixelDepth == 32)
                        a = data.imageData[pixelOffset+3];
					pixelOffset += (header.imageInfo.pixelDepth / 8);
					isPacketRaw = false;
				}
				else
				{
					isPacketRaw = true;
				}
			}
			if (isPacketRaw == true)
			{
				b = data.imageData[pixelOffset];
				g = data.imageData[pixelOffset+1];
				r = data.imageData[pixelOffset+2];
                if (header.imageInfo.pixelDepth == 32)
                    a = data.imageData[pixelOffset+3];
				pixelOffset += (header.imageInfo.pixelDepth / 8);
			}
            bitmap[plotX + plotY * header.imageInfo.width].r = b * (a / 255);
            bitmap[plotX + plotY * header.imageInfo.width].g = g * (a / 255);
            bitmap[plotX + plotY * header.imageInfo.width].b = r * (a / 255);
            bitmap[plotX + plotY * header.imageInfo.width].a = a;
			packetLength--;
		}
	}
}