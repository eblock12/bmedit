#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "color.h"

const int TILE_SIZE = 32;

enum 
{
    TILE_NONE            = 0x00,
    TILE_FLIP_HORIZONTAL = 0x01,
    TILE_FLIP_VERTICAL   = 0x02
};

class Tileset
{
public:
    Tileset(char *filename, HBITMAP bitmap, int width, int height, int pixelDepth);
    ~Tileset();
    void GetTileRect(int tileNum, RECT *r);
    HBITMAP GetBitmap(int attr);
    int GetTileCount() { return numTiles; }
    char *GetShortFilename();
private:
    void Flip(int attrib, Color *buffer, int width, int height, int pixelDepth);

    char *filename;
    HBITMAP bitmap, bitmapFlipHorz, bitmapFlipVert, bitmapFlipHorzVert;
    int numTiles;
    int width;
    int height;
};