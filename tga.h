#ifndef H_TGA
#define H_TGA

#include <cstdio>
#include "color.h"

namespace TGA
{
    class TGAData
    {
    public:
        TGAData() : id(0), colorMapData(0), imageData(0) {}
        unsigned char *id;
        unsigned char *colorMapData;
        unsigned char *imageData;
    };

    class TGAImageInfo
    {
    public:
        unsigned short xOrigin;
        unsigned short yOrigin;
        unsigned short width;
        unsigned short height;
        unsigned char pixelDepth;
        unsigned char descriptor;
    };

    class TGAColorMapInfo
    {
    public:
        unsigned short firstEntryIndex;
        unsigned short length;
        unsigned char entrySize;
    };

    class TGAHeader
    {
    public:
        unsigned char idLength;
        unsigned char colorMapType;
        unsigned char imageType;
        TGAImageInfo imageInfo;
        TGAColorMapInfo colorMapInfo;
    };

    class TGAImage
    {
    public:
        void LoadFromStream(FILE *file, long fileSize);
        void DrawToBitmap(Color *bitmap);
        TGAHeader header;
        TGAData data;
    private:
        void DrawCompressed(Color *bitmap, int startX, int startY, int endX, int endY, int deltaX, int deltaY);
        float Normalize(int value, int min, int max)
        { return ((float)value * (1.0f / ((float)max - (float)min) ) ); }
    };
}

#endif
