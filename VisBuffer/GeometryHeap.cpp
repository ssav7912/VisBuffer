#include "GeometryHeap.h"
#include "ApplicationHelpers.h"
#include "UploadBuffer.h"
#include "Assertions.h"


using namespace ApplicationHelpers;

void GeometryHeap::Create(const std::wstring& name, size_t InitialNumElements)
{
	const size_t VertexHeapSize = InitialNumElements * DEFAULT_VERTEX_BUFFER_SLACK; 

	DefaultVertexHeap.Create(L"Default Vertex Heap", VertexHeapSize);

	const size_t ConstantBufferSize = InitialNumElements * sizeof(MeshConstantBuffer); 
	ConstantBufferHeap.Create(L"Default Constant Buffer Heap", ConstantBufferSize); 
}

void GeometryHeap::BeginAddMesh(const std::vector<Vertex>& vertices, const MeshConstantBuffer& PrimitiveData, ID3D12GraphicsCommandList2* CommandList)
{
	const auto& newVBV = DefaultVertexHeap.AddMesh(vertices, CommandList); 
	const size_t NumMeshes = MeshData.size();

	MeshConstantBuffer* CBMem = static_cast<MeshConstantBuffer*>(ConstantBufferHeap.Map());
	std::memcpy(CBMem + (NumMeshes), &PrimitiveData, sizeof(MeshConstantBuffer));
	ConstantBufferHeap.Unmap(); 
	
	Mesh newMesh = {};
	newMesh.VertexBufferOffset = newVBV.BufferLocation - DefaultVertexHeap.GetGpuVirtualAddress();
	newMesh.VertexBufferSize = newVBV.SizeInBytes;
	newMesh.ConstantBufferOffset = (NumMeshes * sizeof(MeshConstantBuffer));

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBView{};
	CBView.BufferLocation = ConstantBufferHeap.GetGpuVirtualAddress() + newMesh.ConstantBufferOffset;
	CBView.SizeInBytes = sizeof(MeshConstantBuffer); 

	CBVDescriptors.emplace_back(DescriptorHeap->Alloc(1));
	Device->CreateConstantBufferView(&CBView, CBVDescriptors.back());

	MeshData.push_back(newMesh);
}

void GeometryHeap::UpdateMeshConstants(size_t MeshIndex, const MeshConstantBuffer& PrimitiveData)
{
	ASSERT(MeshIndex < MeshData.size() - 1, "MeshIndex must be smaller than the number of meshes in the heap");

	const auto CBOffset = MeshData[MeshIndex].ConstantBufferOffset; 
	const D3D12_GPU_VIRTUAL_ADDRESS CBBasePtr = ConstantBufferHeap.GetGpuVirtualAddress();

	MeshConstantBuffer* CBMem = static_cast<MeshConstantBuffer*>(ConstantBufferHeap.Map());
	std::memcpy(CBMem + CBOffset, &PrimitiveData, sizeof(MeshConstantBuffer)); 
	ConstantBufferHeap.Unmap();
}
