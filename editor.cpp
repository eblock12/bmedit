#include "editor.h"
#include "application.h"

Editor::Editor()
    : level(0), fileName(0), wasSaved(false), isDirty(false)
{
    NewLevel(64, 64);
}

Editor::~Editor()
{
    if (level)
    {
        delete level;
        level = 0;
    }
    if (tilesets.empty() == false)
    {
        for (std::vector<Tileset *>::iterator i = tilesets.begin(); i != tilesets.end(); i++)
            delete *i;
        tilesets.clear();
    }
}

void Editor::NewLevel(int width, int height)
{
    if (level)
    {
        delete level;
        level = 0;
    }
    level = new Level(width, height);
    SetFileName("Untitled.bml");
    wasSaved = false;
    isDirty = false;
}

void Editor::DeleteTileset(Tileset *tileset)
{
    for (std::vector<Tileset *>::iterator i = tilesets.begin(); i != tilesets.end(); i++)
    {
        if (tileset == (*i))
        {
            tilesets.erase(i);
            delete tileset;
            return;
        }
    }
    SetDirty();
}

void Editor::SetFileName(char *newFileName)
{
    if (fileName != 0)
        delete fileName;
    fileName = new char[strlen(newFileName) + 1];
    strcpy(fileName, newFileName);
}

char *Editor::GetShortFileName()
{
    char *shortFile = fileName + strlen(fileName);
    while (*shortFile != '\\' && shortFile >= fileName) shortFile--;
    shortFile++;
    return shortFile;
}

void Editor::FloodWithTile(int layerId, Tile *tile)
{
    for (unsigned int x = 0; x < GetLevel()->GetWidth(); x++)
    {
        for (unsigned int y = 0; y < GetLevel()->GetHeight(); y++)
        {
            GetLevel()->GetTile(layerId, x, y)->tileId = tile->tileId;
            GetLevel()->GetTile(layerId, x, y)->tileSetId = tile->tileSetId;
            GetLevel()->GetTile(layerId, x, y)->tileAttributes = tile->tileAttributes;
        }
    }
    SetDirty();
}

Tileset *Editor::GetTilesetByFilename(char *filename)
{
    for (std::vector<Tileset *>::iterator i = tilesets.begin(); i != tilesets.end(); i++)
    {
        if (!strcmp((*i)->GetShortFilename(), filename))
            return *i;
    }
    return 0;
}

void Editor::LoadLevel()
{
    if (fileName == 0)
        return;

    Application *app = Application::GetSingletonPtr();

    FILE *file = fopen(fileName, "rbS");
    if (file == 0)
    {
        app->ErrorMsg("Failed to read level file from disk.");
        return;
    }

    unsigned char reserved[64];

    char fileId[4];
    fread(&fileId, sizeof(char), 3, file);
    fileId[3] = 0;
    if (strcmp(fileId, "BML"))
    {
        app->ErrorMsg("Invalid BML file");
        fclose(file);
        return;
    }
    
    unsigned char fileVersion;
    fread(&fileVersion, sizeof(char), 1, file);
    if (fileVersion > BML_VERSION)
    {
        app->ErrorMsg("The version of this file is unsupported by this build of BMEdit");
        fclose(file);
        return;
    }

    unsigned int numTilesets;
    fread(&numTilesets, sizeof(unsigned int), 1, file);

    unsigned int layerCount;
    fread(&layerCount, sizeof(unsigned int), 1, file);

    unsigned char bgRed, bgGreen, bgBlue;
    fread(&bgRed, sizeof(unsigned char), 1, file);
    fread(&bgGreen, sizeof(unsigned char), 1, file);
    fread(&bgBlue, sizeof(unsigned char), 1, file);

    fread(&reserved, sizeof(unsigned char), 64, file);

    if (level)
        delete level;

    level = new Level();
    level->SetBackgroundColor(bgRed, bgGreen, bgBlue);

    char *filePath = new char[strlen(fileName) + 1];
    strcpy(filePath, fileName);
    int i = strlen(fileName);
    while (filePath[i] != '\\') i--;
    filePath[i+1] = 0;

    for (unsigned int i = 0; i < numTilesets; i++)
    {
        unsigned int fileNameLength;
        fread(&fileNameLength, sizeof(unsigned int), 1, file);
        char *tilesetFileName = new char[fileNameLength + 1];
        fread(tilesetFileName, sizeof(char), fileNameLength, file);
        tilesetFileName[fileNameLength] = 0;
        char *fullTilesetPath = new char[strlen(filePath) + fileNameLength + 2];
        sprintf(fullTilesetPath, "%s%s", filePath, tilesetFileName);

        FILE *tilesetFile = fopen(fullTilesetPath, "rb");
        if (tilesetFile == 0)
        {
            app->ErrorMsg("Failed to load tileset file '%s'\nThe tilesets used by this level must be in the same directory as the level file itself.", fullTilesetPath);
            fclose(file);
            NewLevel(64, 64);
            return;
        }
        fclose(tilesetFile);

        app->LoadTileset(fullTilesetPath);

        delete fullTilesetPath;
        delete tilesetFileName;

        fread(&reserved, sizeof(unsigned char), 8, file);
    }

    for (unsigned int i = 0; i < layerCount; i++)
    {
        unsigned int layerId;
        fread(&layerId, sizeof(unsigned int), 1, file);
        level->AddLayerSortId(layerId);
    }

    for (unsigned int i = 0; i < layerCount; i++)
    {
        Layer *layer = new Layer;
        unsigned int nameLength;
        fread(&nameLength, sizeof(unsigned int), 1, file);
        layer->name = new char[nameLength + 1];
        layer->name[nameLength] = 0;
        fread(layer->name, sizeof(char), nameLength, file);
        fread(&layer->width, sizeof(unsigned int), 1, file);
        fread(&layer->height, sizeof(unsigned int), 1, file);
        fread(&layer->enableParallax, sizeof(bool), 1, file);
        fread(&layer->parallaxSpeed, sizeof(int), 1, file);
        fread(&layer->enableAutoScroll, sizeof(bool), 1, file);
        fread(&layer->autoScrollXSpeed, sizeof(float), 1, file);
        fread(&layer->autoScrollYSpeed, sizeof(float), 1, file);
        fread(&layer->enableMirrorX, sizeof(bool), 1, file);
        fread(&layer->enableMirrorY, sizeof(bool), 1, file);
        fread(&layer->globalAlpha, sizeof(unsigned char), 1, file);
        layer->isVisible = true;
        fread(&reserved, sizeof(unsigned char), 8, file);
        layer->tiles = new Tile[layer->width * layer->height];
        fread(layer->tiles, sizeof(Tile), layer->width * layer->height, file);
        level->AddLayer(layer);
    }

    fclose(file);
    wasSaved = true;
    isDirty = true;
}

void Editor::SaveLevel()
{
    if (fileName == 0)
        return;

    Application *app = Application::GetSingletonPtr();

    FILE *file = fopen(fileName, "wbS");
    if (file == 0)
    {
        app->ErrorMsg("Failed to create level file on disk.");
        return;
    }

    unsigned int layerCount = level->GetLayerCount();
    unsigned int numTilesets = (unsigned int)GetNumTilesets();
    unsigned char reserved[64];
    memset(reserved, 0, 64);

    unsigned char bgRed = level->GetBackgroundColorR();
    unsigned char bgGreen = level->GetBackgroundColorG();
    unsigned char bgBlue = level->GetBackgroundColorB();

    char fileId[] = "BML";
    fwrite(fileId, sizeof(char), 3, file);
    fwrite(&BML_VERSION, sizeof(char), 1, file);
    fwrite(&numTilesets, sizeof(unsigned int), 1, file);
    fwrite(&layerCount, sizeof(unsigned int), 1, file);
    fwrite(&bgRed, sizeof(unsigned char), 1, file);
    fwrite(&bgGreen, sizeof(unsigned char), 1, file);
    fwrite(&bgBlue, sizeof(unsigned char), 1, file);
    fwrite(&reserved, sizeof(unsigned char), 64, file);

    for (unsigned int i = 0; i < numTilesets; i++)
    {
        Tileset *tileset = GetTileset(i);
        char *fileName = tileset->GetShortFilename();
        unsigned int fileNameLength = (unsigned int)strlen(fileName);
        fwrite(&fileNameLength, sizeof(unsigned int), 1, file);
        fwrite(fileName, sizeof(char), fileNameLength, file);
        fwrite(reserved, sizeof(unsigned char), 8, file);
    }

    //for (int i = level->GetLayerCount() - 1; i >= 0; i--)
    for (unsigned int i = 0; i < level->GetLayerCount(); i++)
    {
        unsigned int layerId = level->GetLayerOrderId(i);
        fwrite(&layerId, sizeof(unsigned int), 1, file);
    }

    for (unsigned int i = 0; i < level->GetLayerCount(); i++)
    {
        Layer *layer = level->GetLayer(level->GetLayerOrderId(i));
        unsigned int nameLength = strlen(layer->name);
        fwrite(&nameLength, sizeof(unsigned int), 1, file);
        fwrite(layer->name, sizeof(char), nameLength, file);
        fwrite(&layer->width, sizeof(unsigned int), 1, file);
        fwrite(&layer->height, sizeof(unsigned int), 1, file);
        fwrite(&layer->enableParallax, sizeof(bool), 1, file);
        fwrite(&layer->parallaxSpeed, sizeof(int), 1, file);
        fwrite(&layer->enableAutoScroll, sizeof(bool), 1, file);
        fwrite(&layer->autoScrollXSpeed, sizeof(float), 1, file);
        fwrite(&layer->autoScrollYSpeed, sizeof(float), 1, file);
        fwrite(&layer->enableMirrorX, sizeof(bool), 1, file);
        fwrite(&layer->enableMirrorY, sizeof(bool), 1, file);
        fwrite(&layer->globalAlpha, sizeof(unsigned char), 1, file);
        fwrite(&reserved, sizeof(unsigned char), 8, file);
        fwrite(layer->tiles, sizeof(Tile), layer->width * layer->height, file);
    }

    fclose(file);

    wasSaved = true;
    isDirty = false;
}