#include "Win32Application.h"
#include "DXApplication.h"
#include "stdafx.h"

HWND Win32Application::hwnd = nullptr;

int Win32Application::Run(DXApplication* inApplication, HINSTANCE hInstance, int nCmdShow)
{
    


    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXApplicationClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0,0, static_cast<LONG>(inApplication->GetWidth()), static_cast<LONG>(inApplication->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindow(windowClass.lpszClassName, inApplication->GetTitle(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        hInstance,
        inApplication); 

    inApplication->OnInit();

    ShowWindow(hwnd, nCmdShow); 

    MSG msg = {};

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    inApplication->OnDestroy();

    return static_cast<char>(msg.wParam); 

}

LRESULT Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto app = reinterpret_cast<DXApplication*>(hWnd, GWLP_USERDATA);

    switch (message)
    {
        case WM_CREATE:
        {
            LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
        }
        return 0;

        case WM_KEYDOWN:
        {
            if (app)
            {
                app->OnKeyDown(static_cast<UINT8>(wParam));
            }
            return 0;
        }
        case WM_KEYUP:
            if (app)
            {
                app->OnKeyUp(static_cast<UINT8>(wParam));
            }
            return 0;
        case WM_PAINT:
            if (app)
            {
                app->OnUpdate();
                app->OnRender();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam); 

}
