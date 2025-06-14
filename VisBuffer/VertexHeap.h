#pragma once
#include "GPUResource.h"
#include "stdafx.h"
#include "ConstantBuffers.h"
#include "UploadBuffer.h"

class VertexHeap : public GPUResource
{
public:
	VertexHeap(ComPtr<ID3D12Device4> device) : GPUResource(device) { UsageState = D3D12_RESOURCE_STATE_COPY_DEST; }

	virtual void Create(const std::wstring& name, size_t initialBufferSize) override;

	const D3D12_VERTEX_BUFFER_VIEW& AddMesh(const std::vector<Vertex>& vertices, ID3D12GraphicsCommandList2* CommandList);


	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBuffers() const { return VBVs; };

	//call to release temporary upload resources once command list is closed
	void Close();


private:
	std::vector<UploadBuffer> UploadBuffers{};
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VBVs;


};