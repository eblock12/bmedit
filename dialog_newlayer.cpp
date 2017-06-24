#include "dialog_newlayer.h"
#include "application.h"

NewLayerDialog::NewLayerDialog()
{
    MakeDialog(Application::GetSingleton().GetInstance(), IDD_NEWLAYER, Application::GetSingleton().GetLayersWindow(), true);
}

void NewLayerDialog::onInit()
{
    editName.SetWindowHandle(GetDlgItem(windowHandle, IDC_NAME));
    editWidth.SetWindowHandle(GetDlgItem(windowHandle, IDC_X));
    editHeight.SetWindowHandle(GetDlgItem(windowHandle, IDC_Y));
    editName.SetText("New Layer");
    editName.SetFocus();
    editName.SelectAllText();
    editWidth.SetText("64");
    editHeight.SetText("64");
}

void NewLayerDialog::onCommand(int id, HWND controlHandle, int code)
{
    switch (id)
    {
        case IDC_OK:
            {
                char buffer[32];
                editWidth.GetText(buffer, 32);
                int width = atoi(buffer);
                editHeight.GetText(buffer, 32);
                int height = atoi(buffer);
                if (width < 1 || height < 1 || width > 65536 || height > 65536)
                {
                    Application::GetSingleton().ErrorMsg(windowHandle, "Invalid layer dimensions specified. You must enter values between 1 and 65536.");
                    return;
                }
                else if (editName.GetTextLength() == 0)
                {
                    Application::GetSingleton().ErrorMsg(windowHandle, "You must designate a name for the new layer.");
                    editName.SetFocus();
                    return;
                }
                else
                {
                    Layer *newLayer = new Layer;
                    newLayer->name = new char[editName.GetTextLength() + 1];
                    editName.GetText(newLayer->name, editName.GetTextLength() + 1);
                    if (!stricmp(newLayer->name, "base"))
                    {
                        Application::GetSingleton().ErrorMsg(windowHandle, "Cannot name a layer 'Base' this name is reserved internally.");
                        delete newLayer;
                        editName.SetFocus();
                        return;
                    }
                    newLayer->width = width;
                    newLayer->height = height;
                    EndDialog(windowHandle, (INT_PTR)newLayer);
                }
            }
            break;
        case IDC_CANCEL:
            EndDialog(windowHandle, 0);
            break;
    }
}