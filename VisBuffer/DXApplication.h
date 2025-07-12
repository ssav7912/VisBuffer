#pragma once
#include <cstdint>
#include <string>
#include "stdafx.h"
#include "Input.h"

class DXApplication
{

public:
	DXApplication(uint32_t width, uint32_t height, std::wstring name);
	virtual ~DXApplication();


	virtual void OnInit() { InputHandler.Initialise(); };
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() { InputHandler.Shutdown();};


	virtual void OnKeyDown(uint8_t key);
	virtual void OnKeyUp(uint8_t key);
	virtual void OnMouseWheel(int16_t wheelDelta);

	uint32_t GetWidth() const { return m_width; };
	uint32_t GetHeight() const { return m_height; };
	const WCHAR* GetTitle() const { return m_title.c_str(); };

	std::wstring GetAssetFullPath(LPCWSTR assetName);
	static void GetHardwareAdapter(_In_ IDXGIFactory1* Factory, _Outptr_result_maybenull_ IDXGIAdapter1** Adapter, bool requestHighPerformanceAdapter = false);

protected:

	void SetCustomWindowText(LPCWSTR text);

	Input InputHandler; 

	uint32_t m_width;
	uint32_t m_height;
	float aspectRatio;
private:

	std::wstring assetPath;
	std::wstring m_title;
	
};

