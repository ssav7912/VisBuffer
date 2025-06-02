#pragma once
#include "stdafx.h"
#include "GPUResource.h"
#include <string>



using namespace Microsoft::WRL;
class UploadBuffer : public GPUResource
{
public:
	UploadBuffer(ComPtr<ID3D12Device> device) : GPUResource(device), GPUResource::UsageState(D3D12_RESOURCE_STATE_GENERIC_READ) {};

	virtual void Create(const std::wstring& name, size_t BufferSize) override;
	
	//TODO: return a view? 
	void* Map(void);

	void Unmap(size_t begin = 0, size_t end = -1);
	
	
};

