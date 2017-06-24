#pragma once

#include <vector>
#include "tileset.h"

using namespace std;

const int BASE_LAYER = 0;

struct Tile
{
    int tileSetId;
    int tileId;
    int tileAttributes;
};

struct Layer
{
    Tile *tiles;
    char *name;
    unsigned char globalAlpha;
    unsigned int width;
    unsigned int height;
    bool isVisible;
    bool enableMirrorX;
    bool enableMirrorY;
    bool enableParallax;
    bool enableAutoScroll;
    int parallaxSpeed;
    float autoScrollXSpeed;
    float autoScrollYSpeed;
};

class Level
{
public:
    Level(int width, int height);
    Level();
    ~Level();
    unsigned int GetWidth() { return layers[BASE_LAYER]->width; }
    unsigned int GetHeight() { return layers[BASE_LAYER]->height; }
    Tile *GetTile(int layerId, int xTile, int yTile);
    Tile *GetTile(int xTile, int yTile);
    void AddLayerSortId(unsigned int layerId) { layerOrder.push_back(layerId); }
    void AddLayer(Layer *newLayer) { layers.push_back(newLayer); }
    Layer *AddLayer(char *name, int width, int height);
    Layer *GetLayer(unsigned int id) { return layers[id]; }
    int GetOrderIdFromLayerId(int layerId);
    void SendLayerBack(int layerOrderId);
    void BringLayerForward(int layerOrderId);
    void DeleteLayer(int layerId);
    unsigned int GetLayerCount() { return (unsigned int)layers.size(); }
    unsigned int GetLayerOrderId(unsigned int i) { return layerOrder[i]; }
    void SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b) { bgRed = r; bgGreen = g; bgBlue = b; }
    unsigned char GetBackgroundColorR() { return bgRed; }
    unsigned char GetBackgroundColorG() { return bgGreen; }
    unsigned char GetBackgroundColorB() { return bgBlue; }
private:
    vector<Layer *>layers;
    vector<int> layerOrder;
    unsigned char bgRed;
    unsigned char bgGreen;
    unsigned char bgBlue;
    int baseLayerId;
};