#include "application.h"

Application::Application(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
    : mainWindowHandle(NULL), editor(NULL)
{
    this->instance = instance;

    INITCOMMONCONTROLSEX cc;
    cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    cc.dwICC = ICC_WIN95_CLASSES;
    if (InitCommonControlsEx(&cc) == FALSE)
    {
        ErrorMsg("Failed to initalize common controls library.");
    }
    
    WNDCLASSEX mainWindowClass;
    ZeroMemory(&mainWindowClass, sizeof(WNDCLASSEX));
    mainWindowClass.cbSize = sizeof(WNDCLASSEX);
    mainWindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
    mainWindowClass.hInstance = instance;
    mainWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    mainWindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    mainWindowClass.lpszClassName = MAIN_WINDOW_CLASS_NAME;
    mainWindowClass.lpfnWndProc = (WNDPROC)MessageHandler;
    mainWindowClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    mainWindowClass.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APPICON));

    if (RegisterClassEx(&mainWindowClass) == 0)
    {
        ErrorMsg("Failed to register window class. The application window cannot be created.");
        initalized = false;
        return;
    }

    int winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;

    if (!(mainWindowHandle = CreateWindowEx(0, MAIN_WINDOW_CLASS_NAME, "BMEdit", winStyle, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, LoadMenu(instance, MAKEINTRESOURCE(IDR_MENU)), instance, NULL)))
    {
        ErrorMsg("The application window could not be created due to an unknown internal error.");
        initalized = false;
        return;
    }

    editor = new Editor();
    SetTitle(editor->GetShortFileName());

    ShowScrollBar(mainWindowHandle, SB_BOTH, true);
    SetupScrollBars();

    selectedLayerId = BASE_LAYER;
    selectedTile.tileId = 0;
    selectedTile.tileSetId = 0;
    selectedTile.tileAttributes = TILE_NONE;
    activeTileset = NULL;

    ShowWindow(mainWindowHandle, cmdShow);

    if (LoadRegSetting("WindowX") != -1 && LoadRegSetting("WindowY") != -1 && LoadRegSetting("WindowWidth") != -1 && LoadRegSetting("WindowHeight") != -1)
    {
        SetWindowPos(mainWindowHandle, 0, LoadRegSetting("WindowX"), LoadRegSetting("WindowY"), LoadRegSetting("WindowWidth"), LoadRegSetting("WindowHeight"),
            SWP_NOZORDER | SWP_NOOWNERZORDER);
    }

    if (LoadRegSetting("WindowMaximized") == 1)
        ShowWindow(mainWindowHandle, SW_MAXIMIZE);

    RECT r;
    GetWindowRect(mainWindowHandle, &r);

    tilesetsDialog = new TilesetsDialog();
    tilesetsDialog->Show();
    tilesetsDialog->SetPosition(r.left, tilesetsDialog->GetPositionY());
    
    layersDialog = new LayersDialog();
    layersDialog->Show();
    layersDialog->SetPosition(r.right - layersDialog->GetWidth(), layersDialog->GetPositionY());

    int gridSetting = LoadRegSetting("ShowGrid");
    HMENU viewMenu = GetSubMenu(GetMenu(mainWindowHandle), 1);

    if (gridSetting == -1 || gridSetting == 1)
    {
        showGrid = true;
        CheckMenuItem(viewMenu, ID_VIEW_SHOWGRID, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        showGrid = false;
        CheckMenuItem(viewMenu, ID_VIEW_SHOWGRID, MF_BYCOMMAND | MF_UNCHECKED);
    }
}

Application::~Application()
{
    if (editor)
    {
        delete editor;
        editor = NULL;
    }
    if (tilesetsDialog)
    {
        tilesetsDialog->Close();
        delete tilesetsDialog;
        tilesetsDialog = NULL;
    }
    if (layersDialog)
    {
        layersDialog->Close();
        delete layersDialog;
        layersDialog = NULL;
    }
}

int Application::Execute()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void Application::ErrorMsg(const char *error, ...)
{
    va_list arg_list;
    va_start(arg_list, error);

    char text[1024];
    vsprintf(text, error, arg_list);
    MessageBox(mainWindowHandle, text, "BMEdit Error", MB_ICONERROR);

    va_end(arg_list);
}

void Application::ErrorMsg(HWND parent, const char *error, ...)
{
    va_list arg_list;
    va_start(arg_list, error);

    char text[1024];
    vsprintf(text, error, arg_list);
    MessageBox(parent, text, "BMEdit Error", MB_ICONERROR);

    va_end(arg_list);
}

void Application::SetTitle(char *text)
{
    char *title = new char[strlen(text) + 32];
    sprintf(title, "BMEdit - %s", text);
    SetWindowText(mainWindowHandle, title);
    delete title;
}

void Application::SetupScrollBars()
{
    SCROLLINFO si;
    ZeroMemory(&si, sizeof(SCROLLINFO));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    
    si.nMax = editor->GetLevel()->GetWidth();
    si.nPage = si.nMax / 8;
    SetScrollInfo(mainWindowHandle, SB_HORZ, &si, true);

    si.nMax = editor->GetLevel()->GetHeight();
    si.nPage = si.nMax / 8;
    SetScrollInfo(mainWindowHandle, SB_VERT, &si, true);
    EnableScrollBar(mainWindowHandle, SB_BOTH, ESB_ENABLE_BOTH);

    scrollX = scrollY = 0;
}

int Application::LoadRegSetting(char *name)
{
	long result;
	int value;
	int size = sizeof(DWORD);
	HKEY hKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_PATH, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	result = RegQueryValueEx(hKey, name, NULL, NULL, (BYTE *)&value, (LPDWORD)&size);
	RegCloseKey(hKey);
	if (result != ERROR_SUCCESS) return -1;
	return value;
}

void Application::SaveRegSetting(char *name, int value)
{
	HKEY hKey;
	RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_PATH, 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegSetValueEx(hKey, name, 0, REG_DWORD, (BYTE *)&value, sizeof(DWORD));
	RegCloseKey(hKey);
}

bool Application::SelectFile(char *title, char *filter, char *fileBuf, int fileBufSize)
{
    char fileTitle[512] = "";
    fileBuf[0] = 0;
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = mainWindowHandle;
    ofn.hInstance = instance;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = fileBuf;
    ofn.nMaxFile = fileBufSize;
    ofn.lpstrFileTitle = fileTitle;
    ofn.nMaxFileTitle = 512;
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING;

    if (!GetOpenFileName(&ofn))
        return false;
    else
        return true;
}

bool Application::SaveFileAs()
{
    char fileName[1024];
    if (SelectFile("Save File As", "Levels (*.bml)\0*.bml\0\0", fileName, 1024) == true)
    {
        char *extension = fileName + strlen(fileName);
        while (*extension != '.')
        {
            extension--;
            if (extension < fileName)
                break;
        }
        extension++;

        if (stricmp(extension, "bml"))
            strcat(fileName, ".bml");

        char *shortFile = fileName + strlen(fileName);
        while (*shortFile != '\\') shortFile--;
        shortFile++;

        editor->SetFileName(fileName);
        SetTitle(editor->GetShortFileName());
        return true;
    }
    else
        return false;
}

bool Application::LoadFile()
{
    char fileName[1024];
    if (SelectFile("Load BML file", "Levels (*.bml)\0*.bml\0\0", fileName, 1024) == true)
    {
        editor->SetFileName(fileName);
        editor->LoadLevel();
        SetTitle(editor->GetShortFileName());
        return true;
    }
    else
        return false;
}

void Application::LoadTileset(char *fileName)
{
    HBITMAP tileSetBmp;
    BITMAP bmInfo;
    Tileset *tileset;

    char *extension = fileName + strlen(fileName);
    while (*extension != '.') extension--;
    extension++;

    if (!stricmp(extension, "tga"))
    {
        TGA::TGAImage tgaImage;
        FILE *file = fopen(fileName, "rb");
        if (!file)
        {
            ErrorMsg("Failed to open '%s'", fileName);
            return;
        }
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fclose(file);
        file = fopen(fileName, "rb");
        tgaImage.LoadFromStream(file, fileSize);
        fclose(file);
        Color *surface = new Color[tgaImage.header.imageInfo.width * tgaImage.header.imageInfo.height];
        tgaImage.DrawToBitmap(surface);
        tileSetBmp = CreateBitmap(tgaImage.header.imageInfo.width, tgaImage.header.imageInfo.height, 1, tgaImage.header.imageInfo.pixelDepth, surface);
        delete[] surface;
        tileset = new Tileset(fileName, tileSetBmp, tgaImage.header.imageInfo.width, tgaImage.header.imageInfo.height, tgaImage.header.imageInfo.pixelDepth);
    }
    else
    {
        tileSetBmp = (HBITMAP)LoadImage(instance, fileName, IMAGE_BITMAP, 256, 256, LR_LOADFROMFILE);
        if (tileSetBmp == NULL)
        {
            ErrorMsg("Failed to load tileset '%s'", fileName);
            return;
        }
        GetObject(tileSetBmp, sizeof(BITMAP), &bmInfo);
        if (bmInfo.bmWidth % TILE_SIZE != 0 || bmInfo.bmHeight % TILE_SIZE != 0 || bmInfo.bmWidth == 0 || bmInfo.bmHeight == 0)
        {
            ErrorMsg("Cannot load tileset, dimensions are not a multiple of %i.", TILE_SIZE);
            return;
        }
        tileset = new Tileset(fileName, tileSetBmp, bmInfo.bmWidth, bmInfo.bmHeight, bmInfo.bmBitsPixel);
    }

    if (editor->isTilesetAvailable() == false)
        activeTileset = tileset;
    editor->AddTileset(tileset);
}

void Application::InsertTileset()
{
    char fileName[1024];
    if (SelectFile("Insert tileset...", "Tilesets (*.tga, *.bmp)\0*.tga;*.bmp\0TGA Files (*.tga)\0*.tga\0Bitmap files (*.bmp)\0*.bmp\0\0", fileName, 1024) == true)
    {
        char *shortFile = fileName + strlen(fileName);
        while (*shortFile != '\\') shortFile--;
        shortFile++;

        if (editor->GetTilesetByFilename(shortFile) != NULL)
        {
            ErrorMsg("A tileset has already been loaded with that filename.");
            return;
        }

        LoadTileset(fileName);

        InvalidateRect(mainWindowHandle, NULL, false);
        tilesetsDialog->UpdateViewer();
    }
}

void Application::DeleteTileset()
{
    if (MessageBox(mainWindowHandle, "This operation will delete the currently selected tileset and invalidate all tiles on the level that use that tileset. Do you wish to proceed?", 
        "BMEdit Warning", MB_ICONWARNING|MB_YESNO) == IDYES)
    {
        editor->DeleteTileset(activeTileset);
        activeTileset = 0;
        if (editor->GetNumTilesets() > 0)
            activeTileset = editor->GetTileset(0);
        tilesetsDialog->UpdateViewer();
        InvalidateRect(mainWindowHandle, NULL, true);
    }
}

void Application::ChooseBackgroundColor()
{
    static COLORREF customColors[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    CHOOSECOLOR cc;
    ZeroMemory(&cc, sizeof(CHOOSECOLOR));
    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.hwndOwner = mainWindowHandle;
    cc.Flags = CC_ANYCOLOR | CC_SOLIDCOLOR | CC_RGBINIT;
    cc.lpCustColors = customColors;
    cc.rgbResult = RGB(editor->GetLevel()->GetBackgroundColorR(), editor->GetLevel()->GetBackgroundColorG(), editor->GetLevel()->GetBackgroundColorB());
    if (ChooseColor(&cc) != 0)
    {
        editor->GetLevel()->SetBackgroundColor(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
        InvalidateRect(mainWindowHandle, 0, true);
        editor->SetDirty();
    }
}

void Application::PlaceTile(int x, int y)
{
    int actualTileSize;
    if (showGrid == true)
        actualTileSize = TILE_SIZE + 1;
    else
        actualTileSize = TILE_SIZE;
    unsigned int tileX = x / actualTileSize + scrollX;
    unsigned int tileY = y / actualTileSize + scrollY;
    if (tileX >= editor->GetLevel()->GetLayer(selectedLayerId)->width || tileY >= editor->GetLevel()->GetLayer(selectedLayerId)->height)
        return;
    Tile *tile = editor->GetLevel()->GetTile(selectedLayerId, tileX, tileY);
    if (tile->tileId != selectedTile.tileId || tile->tileSetId != selectedTile.tileSetId)
    {
        tile->tileId = selectedTile.tileId;
        tile->tileSetId = selectedTile.tileSetId;
        tile->tileAttributes = selectedTile.tileAttributes;
        InvalidateRect(mainWindowHandle, NULL, true);
        editor->SetDirty();
    }
}

void Application::DisplayLayerManagerDialog()
{
    LayersDialog *ld = new LayersDialog();
    ld->Show();
    ld->Loop();
    delete ld;
}

bool Application::FileSaveCheck()
{
    if (editor->isFileDirty())
    {
        char saveCheckMsg[1024];
        sprintf(saveCheckMsg, "The level file '%s' was modified\n\nDo you want to save the changes?", editor->GetShortFileName());
        int retValue = MessageBox(mainWindowHandle, saveCheckMsg, "BMEdit Warning", MB_YESNOCANCEL | MB_ICONWARNING);
        if (retValue == IDYES)
        {
            OnCommand(ID_FILE_SAVELEVEL);
            return true;
        }
        else if (retValue == IDCANCEL)
            return false;
        else
            return true;
    }
    else
        return true;
}

void Application::OnCommand(int id)
{
    switch (id)
    {
        case ID_FILE_NEWLEVEL:
            {
                if (FileSaveCheck() == true)
                {
                    NewLevelDialog *newLevelDialog = new NewLevelDialog();
                    unsigned int retValue = (unsigned int)newLevelDialog->RunDialogBox();
                    if (retValue > 0)
                    {
                        int newHeight = (retValue >> 16);
                        int newWidth = (retValue & 0xFFFF);
                        editor->NewLevel(newWidth, newHeight);
                        SetTitle(editor->GetShortFileName());
                        InvalidateRect(mainWindowHandle, 0, TRUE);
                        SetupScrollBars();
                        tilesetsDialog->UpdateViewer();
                        layersDialog->UpdateLayersList();
                    }
                    delete newLevelDialog;
                }
            }
            break;
        case ID_FILE_LOADLEVEL:
            if (FileSaveCheck() == true)
            {
                LoadFile();
                InvalidateRect(mainWindowHandle, 0, true);
                SetupScrollBars();
                tilesetsDialog->UpdateViewer();
                layersDialog->UpdateLayersList();
            }
            break;
        case ID_FILE_SAVELEVEL:
            if (editor->isSaved())
            {
                editor->SaveLevel();
                break;
            }
        case ID_FILE_SAVEAS:
            if (SaveFileAs() == true)
                editor->SaveLevel();
            break;
        case ID_FILE_EXIT:
            if (FileSaveCheck() == true)
                PostQuitMessage(0);
            break;
        case ID_VIEW_SHOWGRID:
            {
                HMENU viewMenu = GetSubMenu(GetMenu(mainWindowHandle), 1);
                if (showGrid == true)
                {
                    showGrid = false;
                    CheckMenuItem(viewMenu, ID_VIEW_SHOWGRID, MF_BYCOMMAND | MF_UNCHECKED);
                    SaveRegSetting("ShowGrid", 0);
                }
                else
                {
                    showGrid = true;
                    CheckMenuItem(viewMenu, ID_VIEW_SHOWGRID, MF_BYCOMMAND | MF_CHECKED);
                    SaveRegSetting("ShowGrid", 1);
                }
                InvalidateRect(mainWindowHandle, NULL, true);
            }
            break;
        case ID_LEVEL_INSERTTILESET:
            InsertTileset();
            break;
        case ID_LEVEL_MANAGETILES:
            DisplayLayerManagerDialog();
            break;
        case ID_LEVEL_FLOODTILES:
            if (MessageBox(mainWindowHandle, "This operation will set all tiles on this layer to the currently selected tile. Do you wish to proceed?", 
                "BMEdit Warning", MB_ICONWARNING|MB_YESNO) == IDYES)
            {
                editor->FloodWithTile(selectedLayerId, &selectedTile);
                InvalidateRect(mainWindowHandle, NULL, true);
            }
            break;
        case ID_LEVEL_BACKGROUNDCOLOR:
            ChooseBackgroundColor();
            break;
    }
}

void Application::OnLButtonDown(int x, int y)
{
    PlaceTile(x, y);
}

void Application::OnLButtonUp(int x, int y)
{
}

void Application::OnMouseMove(int x, int y)
{
    if (mouseState & MK_LBUTTON)
        PlaceTile(x, y);
}

void Application::OnPaint()
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(mainWindowHandle, &ps);

    HDC tileDc = CreateCompatibleDC(dc);

    RECT r;
    GetClientRect(mainWindowHandle, &r);

    COLORREF bgColor = RGB(editor->GetLevel()->GetBackgroundColorR(), editor->GetLevel()->GetBackgroundColorG(), editor->GetLevel()->GetBackgroundColorB());
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    HPEN bgPen = CreatePen(PS_SOLID, 1, bgColor);

    SelectObject(dc, bgBrush);
    SelectObject(dc, bgPen);
    Rectangle(dc, 0, 0, r.right, r.bottom);

    Level *level = editor->GetLevel();
    unsigned int width = level->GetWidth();
    unsigned int height = level->GetHeight();

    SelectObject(dc, GetStockObject(WHITE_BRUSH));
    SelectObject(dc, GetStockObject(WHITE_PEN));

    RECT tileRect;
    Tile *tile;
    Tileset *tileset;

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    int tileDisplaySize;
    if (showGrid == true)
        tileDisplaySize = TILE_SIZE + 1;
    else
        tileDisplaySize = TILE_SIZE;

    if (editor->isTilesetAvailable())
    {
        unsigned int maxY = scrollY + (r.bottom / tileDisplaySize) + 1;
        if (maxY > height)
            maxY = height;
        unsigned int maxX = scrollX + (r.right / tileDisplaySize) + 1;
        if (maxX > width)
            maxX = width;
        for (unsigned int layerOrderId = 0; layerOrderId < level->GetLayerCount(); layerOrderId++)
        {
            int layerId = level->GetLayerOrderId(layerOrderId);
            Layer *layer = level->GetLayer(layerId);
            if (layer->isVisible == true)
            {
                bf.SourceConstantAlpha = layer->globalAlpha;
                for (unsigned int yTile = scrollY; yTile < maxY; yTile++)
                {
                    for (unsigned int xTile = scrollX; xTile < maxX; xTile++)
                    {
                        int x = xTile * tileDisplaySize - scrollX * tileDisplaySize;
                        int y = yTile * tileDisplaySize - scrollY * tileDisplaySize;
                        tile = level->GetTile(layerId, xTile, yTile);
                        if (tile != 0)
                        {
                            if (tile->tileId != 0)
                            {
                                tileset = editor->GetTileset(tile->tileSetId);
                                tileset->GetTileRect(tile->tileId, &tileRect);
                                SelectObject(tileDc, tileset->GetBitmap(tile->tileAttributes));
                                AlphaBlend(dc, x, y, TILE_SIZE, TILE_SIZE, tileDc, tileRect.left, tileRect.top, TILE_SIZE, TILE_SIZE, bf);
                            }
                        }
                    }
                }
            }
        }
    }

    DeleteObject(bgBrush);
    DeleteObject(bgPen);
    DeleteDC(tileDc);
    EndPaint(mainWindowHandle, &ps);
}

void Application::OnSize(int width, int height)
{
}

void Application::OnScroll(HWND hwnd, int scrollBar, int requestId, int position)
{
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;

    GetScrollInfo(hwnd, scrollBar, &si);

    int lastPos = si.nPos;

    switch (requestId)
    {
        case SB_LINELEFT: 
            si.nPos -= 1;
            break;
        case SB_LINERIGHT: 
            si.nPos += 1;
            break;
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK: 
            si.nPos = si.nTrackPos;
            break;
    }

    SetScrollInfo(hwnd, scrollBar, &si, true);
    GetScrollInfo(hwnd, scrollBar, &si);

    if (hwnd == mainWindowHandle)
    {
        if (scrollBar == SB_HORZ)
            scrollX = si.nPos;
        else if (scrollBar == SB_VERT)
            scrollY = si.nPos;
    }

    if (lastPos != si.nPos)
        InvalidateRect(hwnd, NULL, true);
}


LRESULT Application::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Application *app = Application::GetSingletonPtr();
    switch (msg)
    {
        case WM_CREATE:
            break;
        case WM_CLOSE:
            if (app->FileSaveCheck() == true)
            {
                RECT r;
                WINDOWPLACEMENT wp;
                GetWindowRect(hwnd, &r);
                app->SaveRegSetting("WindowX", r.left);
                app->SaveRegSetting("WindowY", r.top);
                app->SaveRegSetting("WindowWidth", r.right - r.left);
                app->SaveRegSetting("WindowHeight", r.bottom - r.top);
                wp.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(hwnd, &wp);
                app->SaveRegSetting("WindowMaximized", (wp.showCmd & SW_SHOWMAXIMIZED) == SW_SHOWMAXIMIZED ? 1 : 0);
                DestroyWindow(hwnd);
            }
            return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_ERASEBKGND:
            return 0;
        case WM_LBUTTONDOWN:
            Application::GetSingleton().mouseState = (int)wParam;
            Application::GetSingleton().OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_LBUTTONUP:
            Application::GetSingleton().mouseState = (int)wParam;
            Application::GetSingleton().OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOUSEMOVE:
            Application::GetSingleton().mouseState = (int)wParam;
            Application::GetSingleton().OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_SIZE:
            Application::GetSingleton().OnSize(LOWORD(lParam), HIWORD(lParam));
            return DefWindowProc(hwnd, msg, wParam, lParam);
            break;
        case WM_COMMAND:
            Application::GetSingleton().OnCommand(LOWORD(wParam));
            break;
        case WM_PAINT:
            Application::GetSingleton().OnPaint();
            break;
        case WM_HSCROLL:
            Application::GetSingleton().OnScroll(hwnd, SB_HORZ, LOWORD(wParam), HIWORD(wParam));
            break;
        case WM_VSCROLL:
            Application::GetSingleton().OnScroll(hwnd, SB_VERT, LOWORD(wParam), HIWORD(wParam));
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}