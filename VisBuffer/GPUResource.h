#pragma once
#include "stdafx.h"
#include <string>

enum class BufferElem : uint8_t
{
	SCALAR = 1,
	VEC2 = 2,
	VEC3 = 3,
	VEC4 = 4,
};

enum class BufferType : uint8_t
{
	UCHAR = 1,
	USHORT = 2,
	UINT = 3,
	FLOAT = 4, 
};

using namespace Microsoft::WRL; 
struct BufferView
{
	size_t BufferIndex = -1;
	size_t OffsetInBytes = 0;
	size_t LengthInBytes= 0;
	size_t StrideInBytes = 0;
	size_t Nmembers = 0;
	BufferElem ElemType = BufferElem::VEC3;
	BufferType ComponentType = BufferType::FLOAT;

	static constexpr inline DXGI_FORMAT ToDXGIFormat(BufferElem Elem, BufferType Component)
	{
		switch (Elem)
		{
			case BufferElem::SCALAR:
			{
				switch (Component)
				{
					case BufferType::UCHAR: return DXGI_FORMAT_R8_UINT;
					case BufferType::USHORT: return DXGI_FORMAT_R16_UINT;
					case BufferType::UINT: return DXGI_FORMAT_R32_UINT;
					case BufferType::FLOAT: return DXGI_FORMAT_R32_FLOAT;
					default: { ASSERT(false, "switch case leak - should be unreachable"); _STL_UNREACHABLE; };
				}
			}

			case BufferElem::VEC2:
			{
				switch (Component)
				{
					case BufferType::UCHAR: return DXGI_FORMAT_R8G8_UINT;
					case BufferType::USHORT: return DXGI_FORMAT_R16G16_UINT;
					case BufferType::UINT: return DXGI_FORMAT_R32G32_UINT;
					case BufferType::FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
					default: {ASSERT(false, "switch case leak - should be unreachable"); _STL_UNREACHABLE;};
				}
			}
			case BufferElem::VEC3:
			{
				switch (Component)
				{
					case BufferType::UCHAR: { ASSERT(false, "Unsupported resource type combination."); return DXGI_FORMAT_UNKNOWN; };
					case BufferType::USHORT: { ASSERT(false, "Unsupported resource type combination."); return DXGI_FORMAT_UNKNOWN; };
					case BufferType::UINT: return DXGI_FORMAT_R32G32B32_UINT;
					case BufferType::FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
					default: { ASSERT(false, "switch case leak - should be unreachable"); _STL_UNREACHABLE; };

				}
			}
			case BufferElem::VEC4:
			{
				switch (Component)
				{
					case BufferType::UCHAR: return DXGI_FORMAT_R8G8B8A8_UINT;
					case BufferType::USHORT: return DXGI_FORMAT_R16G16B16A16_UINT;
					case BufferType::UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
					case BufferType::FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
					default: { ASSERT(false, "switch case leak - should be unreachable"); _STL_UNREACHABLE; };

				}
			}
			default: _STL_UNREACHABLE;
		}
		ASSERT(false, "switch case leak - Should be unreachable");
	}
};


class GPUResource
{
public:
	GPUResource(ComPtr<ID3D12Device4> device) : Device(device) {};

	virtual void Create(const std::wstring& name, size_t BufferSize) = 0;

	~GPUResource() { Destroy(); };

	ID3D12Resource* GetResource() const { return Resource.Get(); };
	ID3D12Resource** GetAddressOf() { return Resource.GetAddressOf(); };
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return GPUAddress;  };
	uint32_t GetVersionId() const { return versionId; };
	
protected:
	size_t BufferSize = 0;

	ComPtr<ID3D12Device4> Device;
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

