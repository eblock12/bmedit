#pragma once

#include "window.h"

class NewLayerDialog : public Window
{
public:
    NewLayerDialog();
private:
    void onInit();
    void onCommand(int id, HWND controlHandle, int code);

    EditBox editName;
    EditBox editWidth;
    EditBox editHeight;
};