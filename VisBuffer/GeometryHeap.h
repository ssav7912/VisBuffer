#pragma once
#include <vector>
#include "stdafx.h"
#include "directxmath.h"
#include "GPUResource.h"

struct Vertex {
	XMFLOAT3 Position;
	XMFLOAT4 Color; 
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
	GeometryHeap(ComPtr<ID3D12Device> device) : GPUResource(device), GPUResource::UsageState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) {}

	virtual void Create(const std::wstring& name, size_t initialBufferSize) override;

	void AddMesh(std::vector<Vertex>& vertices);


private:
	std::vector<Mesh> MeshData;
};

