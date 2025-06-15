#include "DXApplication.h"
#include "Win32Application.h"

using namespace Microsoft::WRL;

DXApplication::DXApplication(uint32_t width, uint32_t height, std::wstring name) :
	m_width(width), m_height(height), m_title(name)
{
	//TODO: asset paths? 

	aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DXApplication::~DXApplication()
{
}

void DXApplication::OnKeyDown(uint8_t key)
{

}

void DXApplication::OnKeyUp(uint8_t key)
{
}

std::wstring DXApplication::GetAssetFullPath(LPCWSTR assetName)
{
	return assetPath + assetName;
}
_Use_decl_annotations_
void DXApplication::GetHardwareAdapter(IDXGIFactory1* Factory, IDXGIAdapter1** Adapter, bool requestHighPerformanceAdapter)
{
	*Adapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter;

	ComPtr<IDXGIFactory6> factory6;


	if (SUCCEEDED(Factory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (uint32_t adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);


			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue; //skip the software driver (WARP)
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break; //check for d3d12 device support, but don't create it just yet. 
			}

		}
			
	}

	//fallback where no adapter preference interface. 
	if (adapter.Get() == nullptr)
	{
		for (UINT adapterIndex = 0; SUCCEEDED(Factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				// If you want a software adapter, pass in "/warp" on the command line.
				continue;
			}

			// Check to see whether the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
	}

	*Adapter = adapter.Detach(); 
}

void DXApplication::SetCustomWindowText(LPCWSTR text)
{
	std::wstring windowText = m_title + L":" + text; 
	SetWindowText(Win32Application::GetHwnd(), windowText.c_str());

}
