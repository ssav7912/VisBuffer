#pragma once
#include <vector>
#include <directxmath.h>
#include "GPUResource.h"
#include "UploadBuffer.h"
#include "ConstantBuffers.h"
#include "DescriptorHeap.h"
#include "Assertions.h"
#include "VertexHeap.h"
#include "GLTFHeap.h"

struct Mesh
{
	MeshConstantBufferPacked ConstantBuffer; 
	size_t constantBufferIndex = 0;
	size_t NumPrimitives = 0;
	size_t PrimitiveIndex = 0;
};

struct MeshGPUData
{
	uint32_t NumVertices; 
	
	D3D12_GPU_VIRTUAL_ADDRESS VertexBufferOffset;
	D3D12_GPU_VIRTUAL_ADDRESS VertexBufferSize;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBufferOffset;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBufferSize;

	//TODO: move this elsewhere... 
	D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferOffset;
};

class GeometryHeap
{
public: 
	GeometryHeap(ComPtr<ID3D12Device4> device, std::shared_ptr<DescriptorHeap> descriptorHeap) : ConstantBufferHeap(device), DefaultVertexHeap(device), Device(device),
	DescriptorHeap(descriptorHeap), DefaultGLTFHeap(device) {
		ASSERT(descriptorHeap != nullptr, "DescriptorHeap can not be null"); 
		ASSERT(descriptorHeap->GetHeapDescription().Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "Input DescriptorHeap must be for CBVs");
		ASSERT(descriptorHeap->HasSpace(1), "No space left on the heap for additional allocations");

	};

	void Create(const std::wstring& name, size_t InitialNumElements);

	void BeginAddMesh(const std::vector<Vertex>& vertices, const MeshConstantBuffer& PrimitiveData, ID3D12GraphicsCommandList2* CommandList);


	//TODO: Double buffer this... 
	void UpdateMeshConstants(size_t MeshIndex, const MeshConstantBuffer& PrimitiveData);

	const std::vector<MeshGPUData>& GetMeshData() const { return MeshData; };
	const std::vector<D3D12_VERTEX_BUFFER_VIEW>& GetVertexBuffers() const { return DefaultVertexHeap.GetVertexBuffers(); };

	const UploadBuffer& GetMeshConstants() { return ConstantBufferHeap; };

	//call to release temporary upload resources once command list is closed
	void EndAddMesh() { DefaultVertexHeap.Close(); };


	//default amount of space allocated per-object for Create(). 
	static constexpr size_t DEFAULT_VERTEX_BUFFER_SLACK = 256 * sizeof(Vertex);


public: 
	GLTFHeap DefaultGLTFHeap;

private:
	std::vector<DescriptorHandle> CBVDescriptors;
	std::shared_ptr<DescriptorHeap> DescriptorHeap;
	ComPtr<ID3D12Device4> Device;

	//this is really primitive (i.e. per-draw) data
	std::vector<Mesh> MeshCPUData;
	std::vector<MeshGPUData> MeshData;
	UploadBuffer ConstantBufferHeap;
	VertexHeap DefaultVertexHeap; //TODO: Deprecate

};


