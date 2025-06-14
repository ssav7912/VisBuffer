#pragma once
#include <vector>
#include <directxmath.h>
#include "GPUResource.h"
#include "UploadBuffer.h"
#include "ConstantBuffers.h"
#include "DescriptorHeap.h"
#include "Assertions.h"
#include "VertexHeap.h"


struct Mesh
{
	D3D12_GPU_VIRTUAL_ADDRESS VertexBufferOffset;
	D3D12_GPU_VIRTUAL_ADDRESS VertexBufferSize;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBufferOffset;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBufferSize;

	D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferOffset;
};

class GeometryHeap
{
public: 
	GeometryHeap(ComPtr<ID3D12Device4> device, std::shared_ptr<DescriptorHeap> descriptorHeap) : ConstantBufferHeap(device), DefaultVertexHeap(device), Device(device),
	DescriptorHeap(descriptorHeap) {
		ASSERT(descriptorHeap != nullptr, "DescriptorHeap can not be null"); 
		ASSERT(descriptorHeap->GetHeapDescription().Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "Input DescriptorHeap must be for CBVs");
		ASSERT(descriptorHeap->HasSpace(1), "No space left on the heap for additional allocations");

	};

	void Create(const std::wstring& name, size_t InitialNumElements);

	void BeginAddMesh(const std::vector<Vertex>& vertices, const MeshConstantBuffer& PrimitiveData, ID3D12GraphicsCommandList2* CommandList);
	
	//TODO: Double buffer this... 
	void UpdateMeshConstants(size_t MeshIndex, const MeshConstantBuffer& PrimitiveData);

	const std::vector<Mesh>& GetMeshData() const { return MeshData; };
	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBuffers() const { return DefaultVertexHeap.GetVertexBuffers(); };

	const UploadBuffer& GetMeshConstants() { return ConstantBufferHeap; };

	//call to release temporary upload resources once command list is closed
	void EndAddMesh() { DefaultVertexHeap.Close(); };


	//default amount of space allocated per-object for Create(). 
	static constexpr size_t DEFAULT_VERTEX_BUFFER_SLACK = 256 * sizeof(Vertex);

private:
	std::vector<DescriptorHandle> CBVDescriptors;
	std::shared_ptr<DescriptorHeap> DescriptorHeap;
	ComPtr<ID3D12Device4> Device;
	std::vector<Mesh> MeshData;
	UploadBuffer ConstantBufferHeap;
	VertexHeap DefaultVertexHeap;

};


