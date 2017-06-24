#include "dialog_newlevel.h"
#include "application.h"

NewLevelDialog::NewLevelDialog()
{
    MakeDialog(Application::GetSingleton().GetInstance(), IDD_NEW, Application::GetSingleton().GetMainWindow(), true);
}

void NewLevelDialog::onInit()
{
    editWidth.SetWindowHandle(GetDlgItem(windowHandle, IDC_X));
    editHeight.SetWindowHandle(GetDlgItem(windowHandle, IDC_Y));
    editHeight.SetFocus();
    editWidth.SetText("64");
    editHeight.SetText("64");
}

void NewLevelDialog::onCommand(int id, HWND controlHandle, int code)
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
                    Application::GetSingleton().ErrorMsg(windowHandle, "Invalid map dimensions specified. You must enter values between 1 and 65536.");
                    return;
                }
                else
                {
                    unsigned int retValue;
                    retValue = ((unsigned short)width << 16) | (unsigned short)height;
                    EndDialog(windowHandle, retValue);
                }
            }
            break;
        case IDC_CANCEL:
            EndDialog(windowHandle, 0);
            break;
    }
}