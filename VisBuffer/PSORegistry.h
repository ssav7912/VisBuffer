#pragma once
#include "stdafx.h"
#include "FNV1Hash.h"
#include <unordered_map>
class PSORegistry
{
public:

	struct HashedPSODescription
	{
		//!!!MAKE SURE YOUR DESC HAS NO UNITIALISED DATA!!!
		HashedPSODescription(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
	private:
		
		uint64_t hash = 0;
	};

	//use the void argument form of this function to create a TEMPLATE PSO desc.
	//note that in this state, it is not valid!
	static D3D12_GRAPHICS_PIPELINE_STATE_DESC DefaultPSO(ID3DBlob* vertexShader = nullptr, ID3DBlob* pixelShader = nullptr, ID3D12RootSignature* RootSignature = nullptr);
	std::unordered_map<HashedPSODescription, Microsoft::WRL::ComPtr<ID3D12PipelineState>> Registry;

private:
	


};



//extension for std::hash on D3D12_INPUT_LAYOUT_DESC
namespace std
{
	std::hash
}