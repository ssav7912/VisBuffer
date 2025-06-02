#pragma once
#include "stdafx.h"
#include <string>

using namespace Microsoft::WRL; 

class GPUResource
{
public:
	GPUResource(ComPtr<ID3D12Device> device) : Device(device) {};

	virtual void Create(const std::wstring& name, size_t BufferSize) = 0;

	~GPUResource() { Destroy(); };

	ID3D12Resource* GetResource() const { return Resource.Get(); };
	ID3D12Resource** GetAddressOf() { return Resource.GetAddressOf(); };
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return GPUAddress;  };
	uint32_t GetVersionId() const { return versionId; };
	
protected:
	size_t BufferSize = 0;

	ComPtr<ID3D12Device> Device;
	ComPtr<ID3D12Resource> Resource;
	D3D12_RESOURCE_STATES UsageState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	D3D12_RESOURCE_STATES TransitioningState = (D3D12_RESOURCE_STATES)-1;
	D3D12_GPU_VIRTUAL_ADDRESS GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	//used to signal descriptor update on resource update. 
	uint32_t versionId = 0; 

	virtual void Destroy() {
		Resource = nullptr;
		GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		++versionId;
	};
};

