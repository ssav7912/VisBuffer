#pragma once
#include <cstdint>
#include <string>
#include "stdafx.h"

class DXApplication
{

public:
	DXApplication(uint32_t width, uint32_t height, std::wstring name);
	virtual ~DXApplication();


	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;


	virtual void OnKeyDown(uint8_t key) {};
	virtual void OnKeyUp(uint8_t key) {}; 

	uint32_t GetWidth() const { return m_width; };
	uint32_t GetHeight() const { return m_height; };
	const WCHAR* GetTitle() const { return m_title.c_str(); };

	std::wstring GetAssetFullPath(LPCWSTR assetName);

protected:
	void GetHardwareAdapter(_In_ IDXGIFactory1* Factory, _Outptr_result_maybenull_ IDXGIAdapter1** Adapter, bool requestHighPerformanceAdapter = false); 

	void SetCustomWindowText(LPCWSTR text);

	uint32_t m_width;
	uint32_t m_height;
	float aspectRatio;
private:

	std::wstring assetPath;
	std::wstring m_title;
	
};

