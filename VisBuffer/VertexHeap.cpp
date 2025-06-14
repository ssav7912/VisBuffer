#include "VertexHeap.h"
#include "ApplicationHelpers.h"
#include "UploadBuffer.h"
#include "ConstantBuffers.h"
#include "Assertions.h"

using namespace ApplicationHelpers;
void VertexHeap::Create(const std::wstring& name, size_t initialBufferSize)
{

	Destroy();

	BufferSize = initialBufferSize;

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
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


	ThrowIfFailed(Device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &Desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&Resource)));


	GPUAddress = Resource->GetGPUVirtualAddress();

#if defined (_DEBUG)
	Resource->SetName(name.c_str());
#else
	(name);
#endif
}

const D3D12_VERTEX_BUFFER_VIEW& VertexHeap::AddMesh(const std::vector<Vertex>& vertices, ID3D12GraphicsCommandList2* CommandList)
{
	auto& upload = UploadBuffers.emplace_back<UploadBuffer>(Device);
	const size_t VerticesSize = vertices.size() * sizeof(Vertex);

	upload.Create(L"Mesh upload Resource", VerticesSize);

	void* buffer = upload.Map();
	std::memcpy(buffer, vertices.data(), VerticesSize);
	upload.Unmap();

	D3D12_GPU_VIRTUAL_ADDRESS destinationOffset = 0;
	if (VBVs.size() > 0)
	{
		const D3D12_VERTEX_BUFFER_VIEW& lastMesh = VBVs.back();
		destinationOffset = (lastMesh.BufferLocation - GetGpuVirtualAddress()) + lastMesh.SizeInBytes;
	}

	//TODO: expand the heap if mesh is too large. 
	ASSERT((destinationOffset + VerticesSize) < BufferSize, "Size of new mesh overflows buffer.");
	//geometryheap invariant is that resource is only ever used for copy dest or as a vertex buffer
	ASSERT(UsageState == D3D12_RESOURCE_STATE_COPY_DEST || UsageState == D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, "Usage state of buffer is incorrect.");

	if (UsageState != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
		CommandList->ResourceBarrier(1, &Barrier);
	}
	CommandList->CopyBufferRegion(Resource.Get(), destinationOffset, upload.GetResource(), 0, VerticesSize);

	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	UsageState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	CommandList->ResourceBarrier(1, &Barrier);

	D3D12_VERTEX_BUFFER_VIEW VBV = { 0 };
	VBV.BufferLocation = GetGpuVirtualAddress() + destinationOffset;
	VBV.SizeInBytes = VerticesSize;
	VBV.StrideInBytes = sizeof(Vertex);
	VBVs.push_back(VBV);

	return VBVs.back();
}

void VertexHeap::Close()
{
	UploadBuffers.clear();
}
