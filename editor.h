#pragma once

#include <cstring>
#include <vector>

#include "level.h"
#include "tileset.h"

const unsigned char BML_VERSION = 1;

class Editor
{
public:
    Editor();
    ~Editor();
    char *GetFileName() { return fileName; }
    char *GetShortFileName();
    Level *GetLevel() { return level; }
    void AddTileset(Tileset *tileset) { SetDirty(); tilesets.push_back(tileset); }
    void DeleteTileset(Tileset *tileset);
    Tileset *GetTileset(int tileSetId) { return tilesets[tileSetId]; }
    Tileset *GetTilesetByFilename(char *filename);
    int GetNumTilesets() { return (int)tilesets.size(); }
    bool isTilesetAvailable() { return !tilesets.empty(); }
    void FloodWithTile(int layerId, Tile *tile);
    void NewLevel(int width, int height);
    void SetFileName(char *newFileName);
    void SaveLevel();
    void LoadLevel();
    bool isSaved() { return wasSaved; }
    bool isFileDirty() { return isDirty; }
    void SetDirty() { isDirty = true; }
private:
    bool wasSaved;
    bool isDirty;
    Level *level;
    char *fileName;
    std::vector<Tileset *> tilesets;
};