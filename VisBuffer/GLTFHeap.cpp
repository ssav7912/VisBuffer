#include "GLTFHeap.h"
#include "ApplicationHelpers.h"
#include "UploadBuffer.h"

using namespace ApplicationHelpers;
void GLTFHeap::Create(const std::wstring& name, size_t HeapSize)
{
	Destroy();
	BufferSize = HeapSize;

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

D3D12_GPU_VIRTUAL_ADDRESS GLTFHeap::GetAddress(BufferView view) const
{
	ASSERT(view.BufferIndex >= 0 && view.BufferIndex < BufferIndiceOffsets.size(), "View must index into a valid buffer.");
	return Resource->GetGPUVirtualAddress() + BufferIndiceOffsets[view.BufferIndex].Offset;
}

void GLTFHeap::UploadBuffers(const std::vector<tinygltf::Buffer>& buffers, ID3D12GraphicsCommandList2* CommandList)
{
	//ASSERT(UsageState == D3D12_RESOURCE_STATE_COPY_DEST || UsageState == D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, "Usage state of buffer is incorrect.");
	if (UsageState != D3D12_RESOURCE_STATE_COPY_DEST)
	{
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), UsageState, D3D12_RESOURCE_STATE_COPY_DEST);
		CommandList->ResourceBarrier(1, &Barrier);
	}


	for (const auto& buffer : buffers)
	{
		UploadBuffer& Upload = Uploads.emplace_back(Device);
		Upload.Create(std::format(L"GLTF buffer upload resource {}", std::wstring(buffer.name.begin(), buffer.name.end())), buffer.data.size());

		void* uploadptr = Upload.Map(); 
		std::memcpy(uploadptr, buffer.data.data(), buffer.data.size()); 
		Upload.Unmap(); 

		D3D12_GPU_VIRTUAL_ADDRESS destinationOffset = 0;
		if (BufferIndiceOffsets.size() > 0)
		{
			const BufferRegion lastBuffer = BufferIndiceOffsets.back(); 

			destinationOffset = (lastBuffer.Offset) + lastBuffer.SizeInBytes; 


		}
		BufferIndiceOffsets.emplace_back(BufferRegion{ D3D12_RESOURCE_STATE_COMMON, destinationOffset, buffer.data.size() });



		CommandList->CopyBufferRegion(Resource.Get(), destinationOffset, Upload.GetResource(), 0, buffer.data.size());
	}
	

}

void GLTFHeap::EndUploads()
{
	Uploads.clear(); 
}

void GLTFHeap::TransitionState(D3D12_RESOURCE_STATES NewState, ID3D12GraphicsCommandList2* CommandList)
{
	if (UsageState != NewState)
	{
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(), UsageState, NewState);
		CommandList->ResourceBarrier(1, &Barrier); 
		UsageState = NewState; 
	}
}
