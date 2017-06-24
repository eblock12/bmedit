#include "dialog_editlayer.h"
#include "application.h"

EditLayerDialog::EditLayerDialog(Layer *editLayer)
{
    this->editLayer = editLayer;
    MakeDialog(Application::GetSingleton().GetInstance(), IDD_EDITLAYER, Application::GetSingleton().GetLayersWindow(), true);
}

void EditLayerDialog::UpdateControls()
{
    bool isBaseLayer = false;
    if (!strcmp(editLayer->name, "Base"))
        isBaseLayer = true;

    if (checkParallax.isChecked() == true && isBaseLayer == false)
        editParallaxSpeed.Enable();
    else
        editParallaxSpeed.Disable();

    if (checkAutoScroll.isChecked() == true && isBaseLayer == false)
    {
        editAutoScrollSpeedX.Enable();
        editAutoScrollSpeedY.Enable();
    }
    else
    {
        editAutoScrollSpeedX.Disable();
        editAutoScrollSpeedY.Disable();
    }

    if (isBaseLayer == true)
    {
        editName.Disable();
        checkParallax.Disable();
        checkAutoScroll.Disable();
        checkMirrorX.Disable();
        checkMirrorY.Disable();
    }
}

void EditLayerDialog::onInit()
{
    editName.SetWindowHandle(GetDlgItem(windowHandle, IDC_NAME));
    editWidth.SetWindowHandle(GetDlgItem(windowHandle, IDC_X));
    editHeight.SetWindowHandle(GetDlgItem(windowHandle, IDC_Y));
    editParallaxSpeed.SetWindowHandle(GetDlgItem(windowHandle, IDC_PARALLAX_SPEED));
    editAutoScrollSpeedX.SetWindowHandle(GetDlgItem(windowHandle, IDC_AUTOSCROLL_SPEEDX));
    editAutoScrollSpeedY.SetWindowHandle(GetDlgItem(windowHandle, IDC_AUTOSCROLL_SPEEDY));
    editAlpha.SetWindowHandle(GetDlgItem(windowHandle, IDC_ALPHA));
    checkParallax.SetWindowHandle(GetDlgItem(windowHandle, IDC_PARALLAX));
    checkAutoScroll.SetWindowHandle(GetDlgItem(windowHandle, IDC_AUTOSCROLL));
    checkMirrorX.SetWindowHandle(GetDlgItem(windowHandle, IDC_MIRRORX));
    checkMirrorY.SetWindowHandle(GetDlgItem(windowHandle, IDC_MIRRORY));

    editName.SetText(editLayer->name);

    char numeral[32];
    sprintf(numeral, "%i", editLayer->width);
    editWidth.SetText(numeral);

    sprintf(numeral, "%i", editLayer->height);
    editHeight.SetText(numeral);

    sprintf(numeral, "%i", editLayer->parallaxSpeed);
    editParallaxSpeed.SetText(numeral);

    sprintf(numeral, "%.2f", editLayer->autoScrollXSpeed);
    editAutoScrollSpeedX.SetText(numeral);

    sprintf(numeral, "%.2f", editLayer->autoScrollYSpeed);
    editAutoScrollSpeedY.SetText(numeral);

    sprintf(numeral, "%i", (int)((float)editLayer->globalAlpha * (100.0f/255.0f)));
    editAlpha.SetText(numeral);

    if (editLayer->enableParallax)
        checkParallax.SetCheckStatus(true);
    if (editLayer->enableAutoScroll)
        checkAutoScroll.SetCheckStatus(true);
    if (editLayer->enableMirrorX)
        checkMirrorX.SetCheckStatus(true);
    if (editLayer->enableMirrorY)
        checkMirrorY.SetCheckStatus(true);

    UpdateControls();
}

void EditLayerDialog::onCommand(int id, HWND controlHandle, int code)
{
    UpdateControls();
    switch (id)
    {
        case IDC_OK:
            {
                char buffer[32];
                editWidth.GetText(buffer, 32);
                int width = atoi(buffer);
                editHeight.GetText(buffer, 32);
                int height = atoi(buffer);
                editAlpha.GetText(buffer, 32);
                int alpha = atoi(buffer);
                editParallaxSpeed.GetText(buffer, 32);
                int parallaxSpeed = atoi(buffer);
                editAutoScrollSpeedX.GetText(buffer, 32);
                float autoScrollX = (float)atof(buffer);
                editAutoScrollSpeedY.GetText(buffer, 32);
                float autoScrollY = (float)atof(buffer);

                if (width < 1 || height < 1 || width > 65536 || height > 65536)
                    Application::GetSingleton().ErrorMsg(windowHandle, "Invalid layer dimensions specified. You must enter values between 1 and 65536.");
                else if (editName.GetTextLength() == 0)
                {
                    Application::GetSingleton().ErrorMsg(windowHandle, "You must designate a name for this layer.");
                    editName.SetFocus();
                }
                else if (alpha < 0 || alpha > 100)
                {
                    Application::GetSingleton().ErrorMsg(windowHandle, "Invalid global alpha value specified. The value must be between 0 and 100%%.");
                    editAlpha.SetFocus();
                }
                else
                {
                    if (width < (int)editLayer->width || height < (int)editLayer->height)
                    {
                        if (MessageBox(windowHandle, "The specified dimensions are smaller than the current dimensions of this layer.\nResizing the layer will cause the loss of tiles outside the new dimensions.\nDo you wish to continue?", "BMEdit Warning", MB_ICONWARNING|MB_YESNO) != IDYES)
                            return;
                    }
                    if (width != editLayer->width || height != editLayer->height)
                    {
                        Tile *newTiles = new Tile[width * height];
                        memset(newTiles, 0, sizeof(Tile) * width * height);

                        int maxHeight = height > (int)editLayer->height ? (int)editLayer->height : height;
                        int maxWidth = width > (int)editLayer->width ? (int)editLayer->width : width;

                        for (int y = 0; y < maxHeight; y++)
                        {
                            for (int x = 0; x < maxWidth; x++)
                            {
                                newTiles[y * width + x].tileAttributes = editLayer->tiles[y * editLayer->width + x].tileAttributes;
                                newTiles[y * width + x].tileId = editLayer->tiles[y * editLayer->width + x].tileId;
                                newTiles[y * width + x].tileSetId = editLayer->tiles[y * editLayer->width + x].tileSetId;
                            }
                        }
                        editLayer->width = width;
                        editLayer->height = height;
                        delete editLayer->tiles;
                        editLayer->tiles = newTiles;
                    }
                    editLayer->globalAlpha = (unsigned char)((float)alpha * (255.0f/100.0f)) + 1;

                    editLayer->enableParallax = checkParallax.isChecked();
                    editLayer->enableAutoScroll = checkAutoScroll.isChecked();
                    editLayer->enableMirrorX = checkMirrorX.isChecked();
                    editLayer->enableMirrorY = checkMirrorY.isChecked();
                    editLayer->parallaxSpeed = parallaxSpeed;
                    editLayer->autoScrollXSpeed = autoScrollX;
                    editLayer->autoScrollYSpeed = autoScrollY;

                    delete editLayer->name;
                    int nameLength = editName.GetTextLength();
                    editLayer->name = new char[nameLength + 1];
                    editName.GetText(editLayer->name, nameLength + 1);

                    EndDialog(windowHandle, 0);
                }
            }
            break;
        case IDC_CANCEL:
            EndDialog(windowHandle, 0);
            break;
    }
}