#include "DescriptorHeap.h"
#include "ApplicationHelpers.h"
#include "Assertions.h"

void DescriptorHeap::Create(const std::wstring& HeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxDescriptors)
{
	HeapDescription.Type = Type;
	HeapDescription.NumDescriptors = MaxDescriptors;
	HeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HeapDescription.NodeMask = 1;

	ApplicationHelpers::ThrowIfFailed(Device->CreateDescriptorHeap(&HeapDescription, IID_PPV_ARGS(Heap.ReleaseAndGetAddressOf())));

#ifdef _DEBUG
	Heap->SetName(HeapName.c_str());
#endif

	DescriptorSize = Device->GetDescriptorHandleIncrementSize(HeapDescription.Type);
	NumFreeDescriptors = HeapDescription.NumDescriptors;
	FirstHandle = DescriptorHandle(Heap->GetCPUDescriptorHandleForHeapStart(), Heap->GetGPUDescriptorHandleForHeapStart());
	NextFreeHandle = FirstHandle;

		
}

DescriptorHandle DescriptorHeap::Alloc(size_t Count)
{
	ASSERT(HasSpace(Count), "Descriptor Heap is out of space. Increase the Heap size");
	DescriptorHandle ret = NextFreeHandle;
	NextFreeHandle += Count * DescriptorSize;
	NumFreeDescriptors -= Count;
	return ret;
}

bool DescriptorHeap::ValidateHandle(const DescriptorHandle& Handle) const
{
	if (Handle.GetCPUHandle() < FirstHandle.GetCPUHandle() || Handle.GetCPUHandle() >= FirstHandle.GetCPUHandle() + HeapDescription.NumDescriptors * DescriptorSize)
	{
		return false;
	}

	if (Handle.GetGpuHandle() - FirstHandle.GetGpuHandle() != Handle.GetCPUHandle() - FirstHandle.GetCPUHandle())
	{
		return false;
	}

	return true;
}
