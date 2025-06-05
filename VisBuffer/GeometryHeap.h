#pragma once
#include <vector>
#include <directxmath.h>
#include "GPUResource.h"
#include "UploadBuffer.h"

struct Vertex {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color; 
};

struct MeshConstantBuffer
{

};


struct Mesh
{
	D3D12_GPU_VIRTUAL_ADDRESS VertexBufferOffset;
	D3D12_GPU_VIRTUAL_ADDRESS VertexBufferSize;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBufferOffset;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBufferSize;
};

class GeometryHeap : public GPUResource
{
public:
	GeometryHeap(ComPtr<ID3D12Device4> device) : GPUResource(device) { UsageState = D3D12_RESOURCE_STATE_COPY_DEST; }

	virtual void Create(const std::wstring& name, size_t initialBufferSize) override;

	void AddMesh(std::vector<Vertex>& vertices, ID3D12GraphicsCommandList2* CommandList);

	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBuffers() const { return VBVs; };

	//call to release resources once command list is closed
	void Close(); 

private:
	std::vector<UploadBuffer> UploadBuffers{};
	std::vector<Mesh> MeshData;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> VBVs;
};

