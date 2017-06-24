#include "main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Application *app = new Application(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    int retCode = app->Execute();   
    return retCode;
}