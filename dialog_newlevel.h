#pragma once

#include "window.h"

class NewLevelDialog : public Window
{
public:
    NewLevelDialog();
private:
    void onInit();
    void onCommand(int id, HWND controlHandle, int code);

    EditBox editWidth;
    EditBox editHeight;
};