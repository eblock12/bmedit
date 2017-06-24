#pragma once

#include "window.h"

class TilesetListControl : public Control
{
public:
    TilesetListControl();
    static void RegisterClass();
    void SetTileFlipState(bool horizontalFlip, bool verticalFlip) { flipHorizontal = horizontalFlip; flipVertical = verticalFlip; }
private:
    void onInit();
    void onClick(int x, int y);
    void onScroll(int scrollBar, int requestId, int position);
    bool onPaint();
    static bool isRegistered;
    int viewerScroll;
    bool flipHorizontal;
    bool flipVertical;
};

class TilesetsDialog : public Window
{
public:
    TilesetsDialog();
    void UpdateViewer();
private:
    void onInit();
    void onCommand(int id, HWND controlHandle, int code);
    void onResize(int width, int height);

    TilesetListControl tilesetList;
    ComboBox tilesetNameList;
    Button addButton;
    Button deleteButton;
    CheckBox flipHorzCheckbox;
    CheckBox flipVertCheckbox;
};