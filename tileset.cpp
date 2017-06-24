#include "tileset.h"

Tileset::Tileset(char *filename, HBITMAP bitmap, int width, int height, int pixelDepth)
{
    this->filename = new char[strlen(filename) + 1];
    strcpy(this->filename, filename);
    this->bitmap = bitmap;
    this->width = width;
    this->height = height;
    numTiles = (width / TILE_SIZE) * (height / TILE_SIZE);

    Color *buffer = new Color[width * height];
    GetBitmapBits(bitmap, width * height * sizeof(Color), buffer);

    // horizontal flip
    Flip(TILE_FLIP_HORIZONTAL, buffer, width, height, pixelDepth);
    bitmapFlipHorz = CreateBitmap(width, height, 1, pixelDepth, buffer);
    
    // horizontal-vertical flip
    Flip(TILE_FLIP_VERTICAL, buffer, width, height, pixelDepth);
    bitmapFlipHorzVert = CreateBitmap(width, height, 1, pixelDepth, buffer);

    // vertical flip
    Flip(TILE_FLIP_HORIZONTAL, buffer, width, height, pixelDepth);
    bitmapFlipVert = CreateBitmap(width, height, 1, pixelDepth, buffer);

    delete[] buffer;
}

Tileset::~Tileset()
{
    delete filename;
}

void Tileset::Flip(int attrib, Color *buffer, int width, int height, int pixelDepth)
{
    int numBytes = pixelDepth / 8;
    Color *tempBuffer = new Color[width * height];

    for (int yTile = 0; yTile < (height / TILE_SIZE); yTile++)
    {
        for (int xTile = 0; xTile < (width / TILE_SIZE); xTile++)
        {
            for (int y = 0; y < TILE_SIZE; y++)
            {
                for (int x = 0; x < TILE_SIZE; x++)
                {
                    int actualX = x + xTile * TILE_SIZE;
                    int actualY = y + yTile * TILE_SIZE;
                    if ((attrib & TILE_FLIP_HORIZONTAL) == TILE_FLIP_HORIZONTAL)
                    {
                        tempBuffer[actualY * width + xTile * TILE_SIZE + (TILE_SIZE - x - 1)].r = buffer[actualY * width + actualX].r;
                        tempBuffer[actualY * width + xTile * TILE_SIZE + (TILE_SIZE - x - 1)].g = buffer[actualY * width + actualX].g;
                        tempBuffer[actualY * width + xTile * TILE_SIZE + (TILE_SIZE - x - 1)].b = buffer[actualY * width + actualX].b;
                        tempBuffer[actualY * width + xTile * TILE_SIZE + (TILE_SIZE - x - 1)].a = buffer[actualY * width + actualX].a;
                    }
                    else if ((attrib & TILE_FLIP_VERTICAL) == TILE_FLIP_VERTICAL)
                    {
                        tempBuffer[actualX + (yTile * TILE_SIZE + (TILE_SIZE - y - 1)) * width].r = buffer[actualY * width + actualX].r;
                        tempBuffer[actualX + (yTile * TILE_SIZE + (TILE_SIZE - y - 1)) * width].g = buffer[actualY * width + actualX].g;
                        tempBuffer[actualX + (yTile * TILE_SIZE + (TILE_SIZE - y - 1)) * width].b = buffer[actualY * width + actualX].b;
                        tempBuffer[actualX + (yTile * TILE_SIZE + (TILE_SIZE - y - 1)) * width].a = buffer[actualY * width + actualX].a;
                    }
                }
            }
        }
    }

    memcpy(buffer, tempBuffer, width * height * sizeof(Color));
    delete[] tempBuffer;
}

HBITMAP Tileset::GetBitmap(int attr)
{
    if ((attr & TILE_FLIP_HORIZONTAL) == TILE_FLIP_HORIZONTAL && (attr & TILE_FLIP_VERTICAL) == TILE_FLIP_VERTICAL)
        return bitmapFlipHorzVert;
    else if ((attr & TILE_FLIP_HORIZONTAL) == TILE_FLIP_HORIZONTAL)
        return bitmapFlipHorz;
    else if ((attr & TILE_FLIP_VERTICAL) == TILE_FLIP_VERTICAL)
        return bitmapFlipVert;
    else
        return bitmap;
}

void Tileset::GetTileRect(int tileNum, RECT *r)
{
    tileNum--;
    int yTile = tileNum / (width / TILE_SIZE);
    int xTile = tileNum - (yTile * (width / TILE_SIZE));
    r->left = xTile * TILE_SIZE;
    r->top = yTile * TILE_SIZE;
    r->right = r->left + TILE_SIZE;
    r->bottom = r->top + TILE_SIZE;
}

char *Tileset::GetShortFilename()
{
    char *shortFile = filename + strlen(filename);
    while (*shortFile != '\\') shortFile--;
    return shortFile + 1;
}