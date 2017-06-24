#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <map>
#include <cassert>

using namespace std;

LRESULT CALLBACK MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DialogCreationHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

enum msgQueueStatus
{
    QUEUE_EMPTY,
    QUEUE_NOTEMPTY,
    QUEUE_QUIT
};

class Window
{
protected:
    
    HWND dialogParent;
    HDC dcHandle;
    HGLRC rcHandle;
    bool isDialog;
    bool isBlockingDialog;
    UINT dialogResource;
    HINSTANCE dialogInstance;

    virtual void onInit() {}
    virtual void onClose() {}
    virtual void onDestroy() {}
    virtual void onCommand(int id, HWND controlHandle, int code) {}
    virtual void onClick(int x, int y) {}
    virtual void onScroll(int scrollBar, int requestId, int position) {}
    virtual void onResize(int width, int height) {}
    virtual bool onPaint() { return false; }
    virtual void onNotify(int ctrlId, int code) {}
public:
    HWND windowHandle;
    Window() : windowHandle(NULL), dcHandle(NULL), rcHandle(NULL), isDialog(false), isBlockingDialog(false) {}
    ~Window();
    static void Map(HWND winHandle, Window *window);
    static void UnMap(HWND winHandle);
    static Window *GetMap(HWND winHandle);
    void SetFocus() { ::SetFocus(windowHandle); SetActiveWindow(windowHandle); }
    void Show() { ShowWindow(windowHandle, SW_SHOW); }
    void Hide() { ShowWindow(windowHandle, SW_HIDE); }
    void Close() { DestroyWindow(windowHandle); }
    void Update() { UpdateWindow(windowHandle); }
    void Enable() { EnableWindow(windowHandle, TRUE); }
    void Disable() { EnableWindow(windowHandle, FALSE); }
    void Redraw(bool eraseBackground) { InvalidateRect(windowHandle, NULL, eraseBackground); }
    void Redraw(RECT *region, bool eraseBackground) { InvalidateRect(windowHandle, region, eraseBackground); }
    void SetSize(int newWidth, int newHeight) { SetWindowPos(windowHandle, 0, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER); }
    void SetPosition(int newX, int newY) { SetWindowPos(windowHandle, 0, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER); }
    void SetStyle(int styleFlags) { SetWindowLong(windowHandle, GWL_STYLE, styleFlags); }
    int GetPositionX() { RECT r; GetWindowRect(windowHandle, &r); return r.left; }
    int GetPositionY() { RECT r; GetWindowRect(windowHandle, &r); return r.top; }
    int GetWidth() { RECT r; GetWindowRect(windowHandle, &r); return r.right - r.left; }
    int GetHeight() { RECT r; GetWindowRect(windowHandle, &r); return r.bottom - r.top; }
    int GetClientWidth() { RECT r; GetClientRect(windowHandle, &r); return r.right; }
    int GetClientHeight() { RECT r; GetClientRect(windowHandle, &r); return r.bottom; }
    int GetStyle() { return GetWindowLong(windowHandle, GWL_STYLE); }
    msgQueueStatus CheckMessageQueue();
    void Loop();
    void MakeDialog(HINSTANCE instance, UINT resource, HWND parent, bool isBlocking);
    int RunDialogBox();
    virtual LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam);
};

class Control : public Window
{
public:
    void SetWindowHandle(HWND handle);
};

class Button : public Control
{
};

class CheckBox : public Control
{
public:
    bool isChecked() { return SendMessage(windowHandle, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false; }
    void SetCheckStatus(bool checked) { SendMessage(windowHandle, BM_SETCHECK, checked == true ? BST_CHECKED : BST_UNCHECKED, 0); }
};

class EditBox : public Control
{
public:
    int GetTextLength() { return GetWindowTextLength(windowHandle); }
    void GetText(char *buffer, int maxLength) { GetWindowText(windowHandle, buffer, maxLength); }
    void SetText(char *buffer) { SendMessage(windowHandle, WM_SETTEXT, 0, (LPARAM)buffer); }
    void SetSelection(int startPos, int endPos) { SendMessage(windowHandle, EM_SETSEL, startPos, endPos); }
    void SelectAllText() { SetSelection(0, -1); }
};

class ListView : public Control
{
public:
    void DeleteAllItems() { SendMessage(windowHandle, LVM_DELETEALLITEMS, 0, 0); }
    int GetItemCount() { return (int)SendMessage(windowHandle, LVM_GETITEMCOUNT, 0, 0); }
    void InsertColumn(char *text) { InsertColumn(text, 30); }
    void InsertColumn(char *text, int size);
    void InsertColumn(int imageId) { InsertColumn(imageId, 16); }
    void InsertColumn(int imageId, int size);
    int InsertItem(char *text);
    bool isSelected(int itemId) { return SendMessage(windowHandle, LVM_GETITEMSTATE, itemId, LVIS_SELECTED) == 0 ? false : true; }
    bool isChecked(int itemId) { return ListView_GetCheckState(windowHandle, itemId) != 0 ? true : false; }
    void SetImageList(HIMAGELIST imageListHandle, int listType) { SendMessage(windowHandle, LVM_SETIMAGELIST, listType, (LPARAM)imageListHandle); }
    void SetExtendedStyle(int styleFlags) { SendMessage(windowHandle, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, styleFlags); }
    void SetCheckboxState(int itemId, bool checked) { ListView_SetCheckState(windowHandle, itemId, checked); }
    void SelectItem(int itemId, bool select) { ListView_SetItemState(windowHandle, itemId, select == true ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED); }
    void UnselectAll() { ListView_SetItemState(windowHandle, -1, 0, LVIS_SELECTED | LVIS_FOCUSED); }
};

class ComboBox : public Control
{
public:
    int AddString(char *text) { return (int)SendMessage(windowHandle, CB_ADDSTRING, 0, (LPARAM)text); }
    void Clear() { SendMessage(windowHandle, CB_RESETCONTENT, 0, 0); }
    int GetCurrentSelectionId() { return (int)SendMessage(windowHandle, CB_GETCURSEL, 0, 0); }
    int GetItemTextLength(int itemId) { return (int)SendMessage(windowHandle, CB_GETLBTEXTLEN, itemId, 0); }
    void GetItemText(int itemId, char *buffer) { SendMessage(windowHandle, CB_GETLBTEXT, itemId, (LPARAM)buffer); }
    void SetCurrentSelectionId(int id) { SendMessage(windowHandle, CB_SETCURSEL, id, 0); }
};

typedef map<HWND, Window *> mapWindowHandle_t;

static mapWindowHandle_t mapWindowHandle;

