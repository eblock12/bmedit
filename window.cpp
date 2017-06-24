#include "window.h"

Window *newDialog;

LRESULT CALLBACK MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window *w = Window::GetMap(hwnd);

    if (w != NULL)
    {
        return w->MessageHandler(message, wParam, lParam);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK DialogCreationHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    newDialog->windowHandle = hwnd;
    return newDialog->MessageHandler(message, wParam, lParam);
}

Window::~Window()
{
    if (windowHandle)
    {
        DestroyWindow(windowHandle);
    }
}

void Window::Map(HWND winHandle, Window *window)
{
    assert(winHandle != NULL);
    assert(window != NULL);

    mapWindowHandle_t::iterator i = mapWindowHandle.find(winHandle);

    if (i == mapWindowHandle.end())
        mapWindowHandle.insert(mapWindowHandle_t::value_type(winHandle, window));

}

void Window::UnMap(HWND winHandle)
{
    if (!winHandle)
        return;
    
    mapWindowHandle.erase(winHandle);
}

Window *Window::GetMap(HWND winHandle)
{
    assert(winHandle != NULL);

    mapWindowHandle_t::iterator i = mapWindowHandle.find(winHandle);

    if (i != mapWindowHandle.end())
        return i->second;

    return NULL;
}

void Window::MakeDialog(HINSTANCE instance, UINT resource, HWND parent, bool isBlocking)
{
    isBlockingDialog = isBlocking;
    isDialog = true;
    dialogParent = parent;
    dialogInstance = instance;

    if (isBlocking)
    {
        dialogResource = resource;
    }
    else
    {
        windowHandle = CreateDialog(instance, MAKEINTRESOURCE(resource), parent, (DLGPROC)::MessageHandler);
        Map(windowHandle, this);
        SendMessage(windowHandle, WM_INITDIALOG, 0, 0); // message missed, send again
        SendMessage(windowHandle, WM_NCPAINT, 1, 0);
    }
}

int Window::RunDialogBox()
{
    if (isBlockingDialog == true)
    {
        newDialog = this;
        int retValue = (int)DialogBox(dialogInstance, MAKEINTRESOURCE(dialogResource), dialogParent, (DLGPROC)::DialogCreationHandler);
        return retValue;
    }
    else
        return -1;
}

msgQueueStatus Window::CheckMessageQueue()
{
    static MSG msg;

    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT || msg.message == WM_DESTROY || msg.message == WM_CLOSE)
            return QUEUE_QUIT;

        return QUEUE_NOTEMPTY;
    }
    return QUEUE_EMPTY;
}

void Window::Loop()
{
    if (!windowHandle)
        return;
    for (;;)
    {
        if (CheckMessageQueue() == QUEUE_QUIT)
            break;
    }
    while (CheckMessageQueue() != QUEUE_EMPTY);
}

LRESULT Window::MessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT retValue;
    if (isDialog)
        retValue = TRUE;
    else
        retValue = 0;
    switch (message)
    {
        case WM_CREATE:
            onInit();
            return retValue;
        case WM_INITDIALOG:
            onInit();
            return FALSE;
        case WM_CLOSE:
            onClose();
            if (isBlockingDialog == true)
                EndDialog(windowHandle, 0);
            else
                DestroyWindow(windowHandle);
            return retValue;
        case WM_DESTROY:
            onDestroy();
            return retValue;
        case WM_LBUTTONDOWN:
            onClick(LOWORD(lParam), HIWORD(lParam));
            return retValue;
        case WM_HSCROLL:
            onScroll(SB_HORZ, LOWORD(wParam), HIWORD(wParam));
            return retValue;
        case WM_VSCROLL:
            onScroll(SB_VERT, LOWORD(wParam), HIWORD(wParam));
            return retValue;
        case WM_COMMAND:
            onCommand(LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
            return retValue;
        case WM_SIZE:
            onResize(LOWORD(lParam), HIWORD(lParam));
            return retValue;
        case WM_NOTIFY:
            NMHDR *hdr;
            hdr = (NMHDR *)lParam;
            onNotify(hdr->idFrom, hdr->code);
            return retValue;
        case WM_PAINT:
            if (!onPaint())
                return 0;
            return retValue;
    }
    if (isDialog)
        return FALSE;
    else
        return DefWindowProc(windowHandle, message, wParam, lParam);
}

void Control::SetWindowHandle(HWND handle)
{
    UnMap(windowHandle);
    windowHandle = handle;
    Map(windowHandle, this);
}

void ListView::InsertColumn(char *text, int size)
{
    LVCOLUMN lc;
    lc.mask = LVCF_TEXT | LVCF_WIDTH; 
    lc.pszText = text;
    lc.cchTextMax = strlen(text);
    lc.cx = size;
    SendMessage(windowHandle, LVM_INSERTCOLUMN, 0, (LPARAM)&lc);
}

void ListView::InsertColumn(int imageId, int size)
{
    LVCOLUMN lc;
    lc.mask = LVCF_IMAGE | LVCF_WIDTH | LVCF_FMT; 
    lc.fmt = LVCFMT_COL_HAS_IMAGES | LVCFMT_IMAGE;
    lc.iImage = imageId;
    lc.cx = size;
    SendMessage(windowHandle, LVM_INSERTCOLUMN, 0, (LPARAM)&lc);
}

int ListView::InsertItem(char *text)
{
    LVITEM li;
    ZeroMemory(&li, sizeof(LVITEM));
    li.iItem = GetItemCount();
    li.mask = LVIF_TEXT;
    li.pszText = text;
    li.cchTextMax = strlen(text);
    return (int)SendMessage(windowHandle, LVM_INSERTITEM, 0, (LPARAM)&li);
}