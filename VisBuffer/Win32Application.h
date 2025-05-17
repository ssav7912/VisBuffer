#pragma once
#include <wtypes.h>

class DXApplication;


class Win32Application
{

public:
	static int Run(DXApplication* inApplication, HINSTANCE hinstance, int nCmdShow);
	static HWND GetHwnd() { return hwnd; };


protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 
private:
	static HWND hwnd;

	
};

