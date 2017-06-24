#include "tilesets.h"
#include "application.h"

bool TilesetListControl::isRegistered = false;

TilesetListControl::TilesetListControl()
{
    viewerScroll = 0;
    flipHorizontal = false;
    flipVertical = false;
}

void TilesetListControl::onInit()
{
    ShowScrollBar(windowHandle, SB_VERT, true);
    EnableScrollBar(windowHandle, SB_VERT, ESB_DISABLE_BOTH);
}

void TilesetListControl::onClick(int x, int y)
{
    Tileset *activeTileset = Application::GetSingleton().GetActiveTileset();
    Tile *selectedTile = Application::GetSingleton().GetSelectedTile();

    if (activeTileset == NULL)
        return;

    RECT r;
    GetClientRect(windowHandle, &r);
    int startX = r.right / 2 - TILE_SIZE / 2;
    if (x >= startX && x <= startX + TILE_SIZE)
    {
        int tile = (y - 20 + viewerScroll) / (TILE_SIZE + 20);
        if (tile < activeTileset->GetTileCount())
        {
            selectedTile->tileId = tile;
            selectedTile->tileAttributes = TILE_NONE;
            if (flipHorizontal == true)
                selectedTile->tileAttributes |= TILE_FLIP_HORIZONTAL;
            if (flipVertical == true)
                selectedTile->tileAttributes |= TILE_FLIP_VERTICAL;
            InvalidateRect(windowHandle, NULL, false);
        }
    }
}

void TilesetListControl::onScroll(int scrollBar, int requestId, int position)
{
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;

    GetScrollInfo(windowHandle, scrollBar, &si);

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

    SetScrollInfo(windowHandle, scrollBar, &si, true);
    GetScrollInfo(windowHandle, scrollBar, &si);

    viewerScroll = si.nPos;

    if (lastPos != si.nPos)
        InvalidateRect(windowHandle, NULL, true);
}

bool TilesetListControl::onPaint()
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(windowHandle, &ps);
    HDC tileDc = CreateCompatibleDC(dc);

    RECT r;
    GetClientRect(windowHandle, &r);
    SelectObject(dc, GetStockObject(WHITE_BRUSH));
    SelectObject(dc, GetStockObject(NULL_PEN));
    Rectangle(dc, 2, 2, r.right - 1, r.bottom - 1);
    DrawEdge(dc, &r, EDGE_BUMP, BF_RECT);
    
    Application *app = Application::GetSingletonPtr();
    Tileset *activeTileset = app->GetActiveTileset();
    Tile *selectedTile = app->GetSelectedTile();

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    if (activeTileset != NULL)
    {
        bool isSelectedTileset = false;
        if (app->GetEditorInterface()->GetTileset(selectedTile->tileSetId) == activeTileset)
            isSelectedTileset = true;

        RECT tileRect;
        int x = r.right / 2 - TILE_SIZE / 2;
        int y;
        for (int i = 0; i <= activeTileset->GetTileCount(); i++)
        {
            y = i * (TILE_SIZE + 20) + 20 - viewerScroll;
            if (isSelectedTileset == true && selectedTile->tileId == i)
            {
                SelectObject(dc, GetSysColorBrush(COLOR_HIGHLIGHT));
                Rectangle(dc, x - 10, y - 10, x + TILE_SIZE + 10, y + TILE_SIZE + 10);
            }
            if (i == 0)
            {
                HPEN crossPen = CreatePen(PS_SOLID, 5, RGB(230, 40, 40));
                SelectObject(dc, crossPen);
                MoveToEx(dc, x, y, NULL);
                LineTo(dc, x + TILE_SIZE, y + TILE_SIZE);
                MoveToEx(dc, x, y + TILE_SIZE, NULL);
                LineTo(dc, x + TILE_SIZE, y);
                SelectObject(dc, GetStockObject(NULL_PEN));
                DeleteObject(crossPen);
            }
            else
            {
                activeTileset->GetTileRect(i, &tileRect);
                if (flipHorizontal == true && flipVertical == true)
                    SelectObject(tileDc, activeTileset->GetBitmap(TILE_FLIP_HORIZONTAL | TILE_FLIP_VERTICAL));
                else if (flipHorizontal == true)
                    SelectObject(tileDc, activeTileset->GetBitmap(TILE_FLIP_HORIZONTAL));
                else if (flipVertical == true)
                    SelectObject(tileDc, activeTileset->GetBitmap(TILE_FLIP_VERTICAL));
                else
                    SelectObject(tileDc, activeTileset->GetBitmap(TILE_NONE));
                AlphaBlend(dc, x, y, TILE_SIZE, TILE_SIZE, tileDc, tileRect.left, tileRect.top, TILE_SIZE, TILE_SIZE, bf);
            }
        }
    }

    DeleteDC(tileDc);
    EndPaint(windowHandle, &ps);
    return true;
}

void TilesetListControl::RegisterClass()
{
    if (isRegistered == false)
    {
        WNDCLASSEX tilesetViewerClass;
        ZeroMemory(&tilesetViewerClass, sizeof(WNDCLASSEX));
        tilesetViewerClass.cbSize = sizeof(WNDCLASSEX);
        tilesetViewerClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
        tilesetViewerClass.hInstance = Application::GetSingleton().GetInstance();
        tilesetViewerClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        tilesetViewerClass.lpszClassName = TILESET_VIEWER_CLASS_NAME;
        tilesetViewerClass.lpfnWndProc = (WNDPROC)::MessageHandler;
        RegisterClassEx(&tilesetViewerClass);
        isRegistered = true;
    }
}

TilesetsDialog::TilesetsDialog()
{
    TilesetListControl::RegisterClass();
    MakeDialog(Application::GetSingleton().GetInstance(), IDD_TILESETS, Application::GetSingleton().GetMainWindow(), false);
}

void TilesetsDialog::onInit()
{
    addButton.SetWindowHandle(GetDlgItem(windowHandle, IDC_ADD));
    deleteButton.SetWindowHandle(GetDlgItem(windowHandle, IDC_DELETE));
    tilesetNameList.SetWindowHandle(GetDlgItem(windowHandle, IDC_TILESET_LIST));
    tilesetNameList.SetSize(tilesetNameList.GetWidth(), 200);
    tilesetList.SetWindowHandle(GetDlgItem(windowHandle, IDC_TILESET_VIEWER));
    tilesetList.Redraw(true);
    tilesetList.Update();
    flipHorzCheckbox.SetWindowHandle(GetDlgItem(windowHandle, IDC_FLIP_HORZ));
    flipVertCheckbox.SetWindowHandle(GetDlgItem(windowHandle, IDC_FLIP_VERT));
    onResize(GetClientWidth(), GetClientHeight()); 
    UpdateViewer();
}

void TilesetsDialog::onCommand(int id, HWND controlHandle, int code)
{
    if (code == CBN_SELCHANGE)
    {
        int selId = tilesetNameList.GetCurrentSelectionId();
        int bufferLength = tilesetNameList.GetItemTextLength(selId);
        char *buffer = new char[bufferLength + 1];
        tilesetNameList.GetItemText(selId, buffer);
        Tileset *newSet = Application::GetSingleton().GetEditorInterface()->GetTilesetByFilename(buffer);
        if (newSet != NULL)
        {
            Application::GetSingleton().SetActiveTileset(newSet);
            Application::GetSingleton().GetSelectedTile()->tileSetId = selId;
            Application::GetSingleton().GetSelectedTile()->tileId = 0;
            InvalidateRect(windowHandle, NULL, false);
        }
        delete buffer;
    }
    else
    {
        switch (id)
        {
        case IDC_ADD:
            Application::GetSingleton().InsertTileset();
            break;
        case IDC_DELETE:
            Application::GetSingleton().DeleteTileset();
            break;
        case IDC_FLIP_HORZ:
        case IDC_FLIP_VERT:
            {
                tilesetList.SetTileFlipState(flipHorzCheckbox.isChecked(), flipVertCheckbox.isChecked());
                tilesetList.Redraw(true);
                Tile *selectedTile = Application::GetSingleton().GetSelectedTile();
                selectedTile->tileAttributes = TILE_NONE;
                if (flipHorzCheckbox.isChecked() == true)
                    selectedTile->tileAttributes |= TILE_FLIP_HORIZONTAL;
                if (flipVertCheckbox.isChecked() == true)
                    selectedTile->tileAttributes |= TILE_FLIP_VERTICAL;
            }
            break;
        }
    }
}

void TilesetsDialog::onResize(int width, int height)
{
    tilesetNameList.SetSize(width - 20, height);
    addButton.SetPosition(20, 37);
    addButton.SetSize(width / 2 - 20, 21);
    deleteButton.SetPosition(width - 20 - (width / 2) + 20, 37);
    deleteButton.SetSize(width / 2 - 20, 21);
    tilesetList.SetSize(width - 20, height - 140);
    flipHorzCheckbox.SetPosition(width / 2 - flipHorzCheckbox.GetClientWidth() / 2, height - 55);
    flipVertCheckbox.SetPosition(width / 2 - flipVertCheckbox.GetClientWidth() / 2, height - 35);
    UpdateViewer();
}

void TilesetsDialog::UpdateViewer()
{
    HWND hwnd = GetDlgItem(windowHandle, IDC_TILESET_VIEWER);
    RECT r;
    GetClientRect(hwnd, &r);

    Tileset *activeTileset = Application::GetSingleton().GetActiveTileset();

    tilesetList.Redraw(true);

    if (activeTileset == NULL)
    {
        EnableScrollBar(hwnd, SB_VERT, ESB_DISABLE_BOTH);
        tilesetNameList.Clear();
        tilesetNameList.Disable();
        deleteButton.Disable();
        flipHorzCheckbox.Disable();
        flipVertCheckbox.Disable();
        return;
    }
    else if (activeTileset->GetTileCount() * (TILE_SIZE + 20) + 20 > r.bottom)
        EnableScrollBar(hwnd, SB_VERT, ESB_ENABLE_BOTH);

    SCROLLINFO si;
    ZeroMemory(&si, sizeof(SCROLLINFO));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = activeTileset->GetTileCount() * (TILE_SIZE + 20) + 20;
    si.nPage = 4 * (TILE_SIZE + 20);
    SetScrollInfo(hwnd, SB_VERT, &si, true);

    if (Application::GetSingleton().GetEditorInterface()->GetNumTilesets() == 0)
    {
        tilesetNameList.Disable();
        deleteButton.Disable();
        flipHorzCheckbox.Disable();
        flipVertCheckbox.Disable();
    }
    else
    {
        tilesetNameList.Enable();
        deleteButton.Enable();
        flipHorzCheckbox.Enable();
        flipVertCheckbox.Enable();
    }

    tilesetNameList.Clear();
    for (int i = 0; i < Application::GetSingleton().GetEditorInterface()->GetNumTilesets(); i++)
    {
        Tileset *tileset;
        tileset = Application::GetSingleton().GetEditorInterface()->GetTileset(i);
        int id = tilesetNameList.AddString(tileset->GetShortFilename());
        if (tileset == activeTileset)
            tilesetNameList.SetCurrentSelectionId(id);
    }
}