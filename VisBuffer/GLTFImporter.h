#pragma once
#include "tinygltf/tiny_gltf.h"
#include "World/Model.h"
class GLTFImporter
{
public:
	static tinygltf::Model LoadGLB(const std::string& AssetPath);


	static std::vector<std::unique_ptr<Model>> LoadMesh(const tinygltf::Model& model, GeometryHeap* Heap, PSORegistry* Registry, ID3D12GraphicsCommandList2* CommandList);

	static constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertTopologyType(uint32_t TinyGltfModeFlag);

};

