#pragma once 

#include "window.h"
#include "dialog_newlayer.h"
#include "dialog_editlayer.h"

class LayersDialog : public Window
{
public:
    LayersDialog();
    ~LayersDialog();
    void UpdateLayersList();
private:
    void onInit();
    void onCommand(int id, HWND controlHandle, int code);
    void onNotify(int ctrlId, int code);
    int FindSelectedItem();
    int FindSelectedItemOrderId();

    ListView listLayers;
    HIMAGELIST imageList;
    bool disableChecks;
};