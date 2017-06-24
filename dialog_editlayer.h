#pragma once

#include "window.h"
#include "level.h"

class EditLayerDialog : public Window
{
public:
    EditLayerDialog(Layer *editLayer);
private:
    void onInit();
    void onCommand(int id, HWND controlHandle, int code);
    void UpdateControls();

    Layer *editLayer;
    EditBox editName;
    EditBox editWidth;
    EditBox editHeight;
    EditBox editParallaxSpeed;
    EditBox editAutoScrollSpeedX;
    EditBox editAutoScrollSpeedY;
    EditBox editAlpha;
    CheckBox checkParallax;
    CheckBox checkAutoScroll;
    CheckBox checkMirrorX;
    CheckBox checkMirrorY;
};