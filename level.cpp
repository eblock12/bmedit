#include "level.h"
#include "application.h"

Level::Level(int width, int height)
{
    Layer *baseLayer = AddLayer("Base", width, height);
    bgRed = bgGreen = bgBlue = 0;
}

Level::Level()
{
}

Level::~Level()
{
    for (vector<Layer *>::iterator i = layers.begin(); i != layers.end(); i++)
    {
        delete (*i)->tiles;
        delete (*i)->name;
        delete (*i);
    }
}

Tile *Level::GetTile(int layerId, int xTile, int yTile)
{
     if (xTile >= 0 && yTile >= 0 && xTile < (int)layers[layerId]->width && yTile < (int)layers[layerId]->height)
         return &layers[layerId]->tiles[yTile * layers[layerId]->width + xTile];
     else
         return 0;
}

Tile *Level::GetTile(int xTile, int yTile)
{
    return GetTile(BASE_LAYER, xTile, yTile);
}

Layer *Level::AddLayer(char *name, int width, int height)
{
    Layer *newLayer = new Layer;
    newLayer->tiles = new Tile[width * height];
    newLayer->width = width;
    newLayer->height = height;
    newLayer->isVisible = true;
    newLayer->globalAlpha = 255;
    newLayer->name = new char[strlen(name) + 1];
    newLayer->enableAutoScroll = newLayer->enableParallax = newLayer->enableMirrorX = newLayer->enableMirrorY = false;
    newLayer->autoScrollXSpeed = newLayer->autoScrollYSpeed = 0.0f;
    newLayer->parallaxSpeed = 0;
    strcpy(newLayer->name, name);

    for (int i = 0; i < width * height; i++)
    {
        newLayer->tiles[i].tileId = 0;
        newLayer->tiles[i].tileSetId = 0;
        newLayer->tiles[i].tileAttributes = TILE_NONE;
    }

    layers.push_back(newLayer);
    layerOrder.push_back(layers.size() - 1);

    if (!strcmp(newLayer->name, "Base"))
        baseLayerId = layers.size() - 1;

    return newLayer;
}

void Level::DeleteLayer(int layerId)
{
    Layer *layer = GetLayer(layerId);
    for (std::vector<Layer *>::iterator i = layers.begin(); i != layers.end(); i++)
    {
        if (layer == (*i))
        {
            layers.erase(i);
            delete layer;
            break;
        }
    }
    for (std::vector<int>::iterator i = layerOrder.begin(); i != layerOrder.end(); i++)
    {
        if (layerId == (*i))
        {
            layerOrder.erase(i);
            break;
        }
    }
}

void Level::SendLayerBack(int layerOrderId)
{
    if (layerOrderId > 0)
    {
        int saveId = layerOrder[layerOrderId - 1];
        layerOrder[layerOrderId - 1] = layerOrder[layerOrderId];
        layerOrder[layerOrderId] = saveId;
    }
}

void Level::BringLayerForward(int layerOrderId)
{
    if (layerOrderId < ((int)layerOrder.size() - 1))
    {
        int saveId = layerOrder[layerOrderId + 1];
        layerOrder[layerOrderId + 1] = layerOrder[layerOrderId];
        layerOrder[layerOrderId] = saveId;
    }
}

int Level::GetOrderIdFromLayerId(int layerId)
{
    for (int i = 0; i < (int)layerOrder.size(); i++)
    {
        if (layerId == layerOrder[i])
            return i;
    }
    return -1;
}