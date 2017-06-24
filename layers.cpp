#include "layers.h"
#include "application.h"

LayersDialog::LayersDialog()
{
    MakeDialog(Application::GetSingleton().GetInstance(), IDD_LAYERS, Application::GetSingleton().GetMainWindow(), false);

    imageList = ImageList_Create(16, 16, ILC_COLOR4, 1, 0);
    ImageList_AddIcon(imageList, LoadIcon(Application::GetSingleton().GetInstance(), MAKEINTRESOURCE(IDI_LAYER_VISIBLE)));
    disableChecks = false;
}

LayersDialog::~LayersDialog()
{
    ImageList_Destroy(imageList);
}

void LayersDialog::onInit()
{
    listLayers.SetWindowHandle(GetDlgItem(windowHandle, IDC_LAYERLIST));
    listLayers.SetStyle(listLayers.GetStyle() | LBS_NOTIFY);
    listLayers.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES );
    listLayers.InsertColumn("Name", listLayers.GetClientWidth());
    UpdateLayersList();
}

void LayersDialog::onCommand(int id, HWND controlHandle, int code)
{
    if (code == LBN_SELCHANGE)
    {
        Application::GetSingleton().ErrorMsg("pewp");
    }
    else
    {
        switch (id)
        {
            case IDC_ADDLAYER:
                {
                    NewLayerDialog *newLayerDialog = new NewLayerDialog();
                    Layer *newLayer = (Layer *)newLayerDialog->RunDialogBox();
                    delete newLayerDialog;
                    if (newLayer != 0)
                    {
                        Application::GetSingleton().GetEditorInterface()->SetDirty();
                        Application::GetSingleton().GetEditorInterface()->GetLevel()->AddLayer(newLayer->name, newLayer->width, newLayer->height);
                        delete newLayer->name;
                        delete newLayer;
                        UpdateLayersList();
                    }
                }
                break;
            case IDC_PROPERTIES:
                {
                    int layerId = FindSelectedItem();
                    if (layerId != -1)
                    {
                        Application::GetSingleton().GetEditorInterface()->SetDirty();
                        EditLayerDialog *editLayerDialog = new EditLayerDialog(Application::GetSingleton().GetEditorInterface()->GetLevel()->GetLayer(layerId));
                        editLayerDialog->RunDialogBox();
                        delete editLayerDialog;
                        UpdateLayersList();
                        InvalidateRect(Application::GetSingleton().GetMainWindow(), NULL, true);
                    }
                }
                break;
            case IDC_DELETE:
                {
                    int layerId = FindSelectedItem();
                    if (layerId == BASE_LAYER)
                        Application::GetSingleton().ErrorMsg("The base layer cannot be deleted.");
                    else if (layerId != -1)
                    {
                        if (MessageBox(Application::GetSingleton().GetMainWindow(), "This operation will permanetly delete the currently selected layer. Do you wish to proceed?", 
                            "BMEdit Warning", MB_ICONWARNING|MB_YESNO) == IDYES)
                        {
                            Application::GetSingleton().GetEditorInterface()->SetDirty();
                            Application::GetSingleton().GetEditorInterface()->GetLevel()->DeleteLayer(layerId);
                            InvalidateRect(Application::GetSingleton().GetMainWindow(), NULL, true);
                            UpdateLayersList();
                        }
                    }
                    Application::GetSingleton().SetSelectedLayerId(0);
                }
                break;
            case IDC_MOVEUP:
                {
                    int layerId = FindSelectedItemOrderId();
                    if (layerId != -1 && layerId != 0)
                    {
                        Application::GetSingleton().GetEditorInterface()->SetDirty();
                        Application::GetSingleton().GetEditorInterface()->GetLevel()->SendLayerBack(layerId);
                    }
                    UpdateLayersList();
                    InvalidateRect(Application::GetSingleton().GetMainWindow(), NULL, true);
                }
                break;
            case IDC_MOVEDOWN:
                {
                    int layerId = FindSelectedItemOrderId();
                    if (layerId != -1 && layerId < (listLayers.GetItemCount() - 1))
                    {
                        Application::GetSingleton().GetEditorInterface()->SetDirty();
                        Application::GetSingleton().GetEditorInterface()->GetLevel()->BringLayerForward(layerId);
                    }
                    UpdateLayersList();
                    InvalidateRect(Application::GetSingleton().GetMainWindow(), NULL, true);
                }
                break;
        }
    }
}

void LayersDialog::onNotify(int ctrlId, int code)
{
    if (ctrlId == IDC_LAYERLIST)
    {
        if (code == LVN_ITEMCHANGED)
        {
            Level *level = Application::GetSingleton().GetEditorInterface()->GetLevel();
            int itemId = FindSelectedItem();
            if (itemId != -1)
                Application::GetSingleton().SetSelectedLayerId(itemId);
            else if (disableChecks == false)
            {
                //listLayers.UnselectAll();
                //listLayers.SelectItem(level->GetOrderIdFromLayerId(Application::GetSingleton().GetSelectedLayerId()), true);
            }
            for (int i = 0; i < listLayers.GetItemCount(); i++)
            {
                int layerId = level->GetLayerOrderId(i);
                Layer *layer = level->GetLayer(layerId);
                if (disableChecks == false)
                {
                    if (listLayers.isChecked(i))
                        layer->isVisible = true;
                    else
                        layer->isVisible = false;
                }
            }
            InvalidateRect(Application::GetSingleton().GetMainWindow(), NULL, true);
        }
    }
}

int LayersDialog::FindSelectedItem()
{
    for (int i = 0; i < listLayers.GetItemCount(); i++)
    {
        if (listLayers.isSelected(i))
            return Application::GetSingleton().GetEditorInterface()->GetLevel()->GetLayerOrderId(i);
    }
    return -1;
}

int LayersDialog::FindSelectedItemOrderId()
{
    for (int i = 0; i < listLayers.GetItemCount(); i++)
    {
        if (listLayers.isSelected(i))
            return i;
    }
    return -1;
}


void LayersDialog::UpdateLayersList()
{
    listLayers.DeleteAllItems();

    Level *level = Application::GetSingleton().GetEditorInterface()->GetLevel();

    disableChecks = true;
    for (unsigned int i = 0; i < level->GetLayerCount(); i++)
    {
        Layer *layer = level->GetLayer(level->GetLayerOrderId(i));
        int itemId = listLayers.InsertItem(layer->name);
        if (layer->isVisible == true)
            listLayers.SetCheckboxState(itemId, true);
    }

    if (FindSelectedItem() == -1)
    {
        listLayers.UnselectAll();
        listLayers.SelectItem(level->GetOrderIdFromLayerId(Application::GetSingleton().GetSelectedLayerId()), true);
    }

    disableChecks = false;
}