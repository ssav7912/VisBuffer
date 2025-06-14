#pragma once
#include "stdafx.h"

using namespace Microsoft::WRL;

//encapsulates a handle to a shader visible descriptor
class DescriptorHandle
{
public:

	DescriptorHandle() 
	{
		CPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		GPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	};

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CPU, D3D12_GPU_DESCRIPTOR_HANDLE GPU): CPUHandle(CPU), GPUHandle(GPU) {};

	DescriptorHandle operator+ (int64_t Offset) const
	{
		auto ret = *this;
		ret += Offset;
		return ret;
	}

	void operator+= (int64_t Offset)
	{
		if (CPUHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			CPUHandle.ptr += Offset;
		}

		if (GPUHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			GPUHandle.ptr += Offset; 
		}
	
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE* operator&() const { return &CPUHandle; };
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return CPUHandle; };
	operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return GPUHandle; };

	size_t GetCPUHandle() const{ return CPUHandle.ptr; };
	size_t GetGpuHandle() const { return GPUHandle.ptr; }; 
	bool isNull() const { return CPUHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN || CPUHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL; };
	bool isShaderVisible() const { return GPUHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; };

private:
	D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle; 
	D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;
};


class DescriptorHeap
{

public:
	DescriptorHeap(ComPtr<ID3D12Device4> device) : Device(device) {};

	void Create(const std::wstring& HeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxDescriptors);

	void Destroy() { Heap.Reset(); };

	bool HasSpace(size_t Count) const { return Count <= NumFreeDescriptors;  };
	DescriptorHandle Alloc(size_t Count = 1); 

	DescriptorHandle operator[] (size_t arrayIndex) const { return FirstHandle + arrayIndex * DescriptorSize; };

	size_t GetHandleOffset(const DescriptorHandle& Handle) const {
		return (Handle.GetCPUHandle() - FirstHandle.GetCPUHandle()) / DescriptorSize;
	}

	bool ValidateHandle(const DescriptorHandle& Handle) const;

	size_t GetDescriptorSize() const { return DescriptorSize; };

	const D3D12_DESCRIPTOR_HEAP_DESC& GetHeapDescription() const { return HeapDescription; };

private:
	ComPtr<ID3D12Device4> Device;
	ComPtr<ID3D12DescriptorHeap> Heap;
	D3D12_DESCRIPTOR_HEAP_DESC HeapDescription = {};
	size_t DescriptorSize = 0;
	size_t NumFreeDescriptors = 0;
	DescriptorHandle FirstHandle;
	DescriptorHandle NextFreeHandle; 


};

