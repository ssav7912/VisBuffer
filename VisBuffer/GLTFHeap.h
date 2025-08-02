#pragma once
#include "GPUResource.h"
#include "stdafx.h"
#include "UploadBuffer.h"
#include "tinygltf/tiny_gltf.h"


class GLTFHeap : GPUResource
{
public:
	GLTFHeap(ComPtr<ID3D12Device4> device) : GPUResource(device) { UsageState = D3D12_RESOURCE_STATE_COPY_DEST; };



	virtual void Create(const std::wstring& name, size_t HeapSize) override;

	D3D12_GPU_VIRTUAL_ADDRESS GetAddress(BufferView view) const;
	void UploadBuffers(const std::vector<tinygltf::Buffer>& buffers, ID3D12GraphicsCommandList2* CommandList); 
	void EndUploads();
	void TransitionState(D3D12_RESOURCE_STATES NewState, ID3D12GraphicsCommandList2* CommandList); 

	D3D12_GRAPHICS_PIPELINE_STATE_DESC BasePSO(ID3DBlob* vertexShader, ID3DBlob* pixelShader); 

private:
	struct BufferRegion
	{
		D3D12_RESOURCE_STATES UsageState;
		size_t Offset;
		size_t SizeInBytes;
	};
	//bufferIndex in BufferView struct retrieves the offset into the concatenated heap for the desired buffer 
	std::vector<BufferRegion> BufferIndiceOffsets;
	std::vector<UploadBuffer> Uploads; 
};

