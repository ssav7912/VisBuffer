#pragma once
#include "stdafx.h"

using namespace Microsoft::WRL;

//encapsulates a handle to a shader visible descriptor
class DescriptorHandle
{
	friend class DescriptorHeap;
public:

	DescriptorHandle() 
	{
		CPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		GPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		numDescriptors = 0;
	};

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE CPU, D3D12_GPU_DESCRIPTOR_HANDLE GPU, uint32_t numDescriptors = 1): CPUHandle(CPU), GPUHandle(GPU), numDescriptors(numDescriptors) {};

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

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return CPUHandle; };
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return GPUHandle; };
	size_t GetCPUHandlePtr() const{ return CPUHandle.ptr; };
	size_t GetGpuHandlePtr() const { return GPUHandle.ptr; }; 
	bool isNull() const { return CPUHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN || CPUHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_NULL; };
	bool isShaderVisible() const { return GPUHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; };
protected:
	uint32_t numDescriptors;
private:
	D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle; 
	D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;
};


class DescriptorHeap final
{
	//imgui needs to see our descriptor heap, so
	//this allows us to keep the interface private. 
	friend class ImGuiHelper;
	friend class DescriptorHeapTests; 


public:
	DescriptorHeap(ComPtr<ID3D12Device4> device) : Device(device) {};
	DescriptorHeap(const DescriptorHeap&) = delete;

	void Create(const std::wstring& HeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxDescriptors);

	//TODO: more robust destroy semantics...
	void Destroy() { Heap.Reset(); FreeIndices.clear(); FirstHandle = DescriptorHandle(); };

	bool HasSpace(size_t Count) const { return Count <= NumFreeDescriptors;  };
	DescriptorHandle Alloc(uint32_t Count = 1); 
	void Free(DescriptorHandle& Handle);

	DescriptorHandle operator[] (size_t arrayIndex) const { return FirstHandle + arrayIndex * DescriptorSize; };

	size_t GetHandleOffset(const DescriptorHandle& Handle) const {
		return (Handle.GetCPUHandlePtr() - FirstHandle.GetCPUHandlePtr()) / DescriptorSize;
	}

	bool ValidateHandle(const DescriptorHandle& Handle) const;

	size_t GetDescriptorSize() const { return DescriptorSize; };

	const D3D12_DESCRIPTOR_HEAP_DESC& GetHeapDescription() const { return HeapDescription; };

protected:
	//support utility for IMGui. Should not typically be used!
	ID3D12DescriptorHeap* GetHeap() const { return Heap.Get(); };

private:
	struct BlockIndex { uint32_t offset; uint32_t numDescriptors; };

	ComPtr<ID3D12Device4> Device;
	ComPtr<ID3D12DescriptorHeap> Heap;
	/*offsets from FirstHandle
	array based free-list implementation.
	block neighbours are index neighbours in the vector.
	blocks are stored in address order. */
	std::vector<BlockIndex> FreeIndices; 
	D3D12_DESCRIPTOR_HEAP_DESC HeapDescription = {};
	size_t DescriptorSize = 0;
	size_t NumFreeDescriptors = 0;
	DescriptorHandle FirstHandle;



};

