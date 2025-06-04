#include "stdafx.h"
#include "VisBuffer.h"
#include "Win32Application.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	VisBuffer program(1280, 720, L"D3D12 Visibility Buffer");
	return Win32Application::Run(&program, hInstance, nCmdShow);
}