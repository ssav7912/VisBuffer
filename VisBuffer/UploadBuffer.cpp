
#define NOMINMAX
#include <algorithm>
#include "UploadBuffer.h"
#include "ApplicationHelpers.h"
using namespace ApplicationHelpers;


void UploadBuffer::Create(const std::wstring& name, size_t BufferSize)
{

    Destroy();

    BufferSize = BufferSize; 

    D3D12_HEAP_PROPERTIES HeapProps;
    HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProps.CreationNodeMask = 1;
    HeapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC Desc = {}; 
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = BufferSize;
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.SampleDesc.Quality = 0;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    Desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    //TODO: device global...? 
    ThrowIfFailed(Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &Desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Resource))); 

    GPUAddress = Resource->GetGPUVirtualAddress(); 

#if defined (_DEBUG)
    Resource->SetName(name.c_str());
#else
    (name);
#endif

}

void* UploadBuffer::Map(void)
{
    void* memory;
    Resource->Map(0, &CD3DX12_RANGE(0, BufferSize), &memory);
    return memory;
}

void UploadBuffer::Unmap(size_t begin, size_t end)
{

    Resource->Unmap(0, &CD3DX12_RANGE(begin, std::min(end, BufferSize))); 
}
