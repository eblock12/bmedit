#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Commdlg.h>
#include <commctrl.h>
#include <cstdarg>
#include <cstdio>

#include "resource.h"
#include "singleton.h"
#include "editor.h"
#include "layers.h"
#include "tilesets.h"
#include "dialog_newlevel.h"
#include "tga.h"

const char MAIN_WINDOW_CLASS_NAME[] = "BMEDIT_MAIN_WINDOW_CLASS";
const char TILESET_VIEWER_CLASS_NAME[] = "TILESET_VIEWER_CLASS";
const char REG_KEY_PATH[] = "Software\\BMEdit";

class Application : public Singleton<Application>
{
public:
    Application(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow);
    ~Application();
    int Execute();
    void ErrorMsg(HWND parent, const char *error, ...);
    void ErrorMsg(const char *error, ...);
    HINSTANCE GetInstance() { return instance; }
    HWND GetMainWindow() { return mainWindowHandle; }
    HWND GetLayersWindow() { return layersDialog->windowHandle; }

    void InsertTileset();
    void LoadTileset(char *fileName);
    void DeleteTileset();
    bool LoadFile();
    bool SaveFileAs();
    void ChooseBackgroundColor();

    int LoadRegSetting(char *name);
    void SaveRegSetting(char *name, int value);

    void SetActiveTileset(Tileset *newSet) { activeTileset = newSet; tilesetsDialog->UpdateViewer(); }
    void SetSelectedLayerId(int layerId) { selectedLayerId = layerId; }
    Tileset *GetActiveTileset() { return activeTileset; }
    Editor *GetEditorInterface() { return editor; }
    Tile *GetSelectedTile() { return &selectedTile; }
    int GetSelectedLayerId() { return selectedLayerId; }
private:
    static LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void SetTitle(char *text);
    void SetupScrollBars();
    bool SelectFile(char *title, char *filter, char *fileBuf, int fileBufSize);
    void PlaceTile(int x, int y);

    void DisplayLayerManagerDialog();
    bool FileSaveCheck();

    // main window events
    void OnPaint();
    void OnCommand(int id);
    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);
    void OnMouseMove(int x, int y);
    void OnSize(int width, int height);
    void OnScroll(HWND hwnd, int scrollBar, int requestId, int position);

    TilesetsDialog *tilesetsDialog;
    LayersDialog *layersDialog;

    bool initalized;
    bool showGrid;
    HINSTANCE instance;
    HWND mainWindowHandle;

    int mouseState;

    int scrollX;
    int scrollY;

    int selectedLayerId;
    Tile selectedTile;
    Tileset *activeTileset;

    Editor *editor;

};