#include "PSORegistry.h"
#include <span>
#include <array>
#include <cstddef>

D3D12_GRAPHICS_PIPELINE_STATE_DESC PSORegistry::DefaultPSO(ID3DBlob* vertexShader = nullptr, ID3DBlob* pixelShader = nullptr, ID3D12RootSignature* RootSignature = nullptr)
{

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODescription = {};
	PSODescription.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	PSODescription.pRootSignature = RootSignature;
	PSODescription.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	PSODescription.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	PSODescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PSODescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	PSODescription.DepthStencilState.DepthEnable = false;
	PSODescription.DepthStencilState.StencilEnable = false;
	PSODescription.SampleMask = UINT_MAX;
	PSODescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PSODescription.NumRenderTargets = 1;
	PSODescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PSODescription.SampleDesc.Count = 1;
	return PSODescription;
}

PSORegistry::HashedPSODescription::HashedPSODescription(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
	//serialise a PSO description to a buffer, ignoring padding. Then hash that buffer.
	//special cases Input layout, where instead of writing the raw pointers (which will differ even with identical layouts due to allocation),
	//the layout array is walked and each element appended inline
	//TODO: do the same for the RootSignature? 
	//TODO: be smart and ignore properties if certain features are disabled
	std::vector<std::byte> serialisedDescription;
	serialisedDescription.reserve(sizeof(desc));
	
	serialisedDescription.append_range(FNV1::makeSpanView(desc.pRootSignature)); 
	serialisedDescription.append_range(FNV1::makeSpanView(desc.VS));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.PS));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DS));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.HS));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.GS));

	//stream output
	serialisedDescription.append_range(FNV1::makeSpanView(desc.StreamOutput.pSODeclaration)); 
	serialisedDescription.append_range(FNV1::makeSpanView(desc.StreamOutput.NumEntries));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.StreamOutput.pBufferStrides));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.StreamOutput.NumStrides));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.StreamOutput.RasterizedStream)); 

	//Blend state
	serialisedDescription.append_range(FNV1::makeSpanView(desc.BlendState.AlphaToCoverageEnable));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.BlendState.IndependentBlendEnable));
	//blend state RT blend desc
	for (const auto& blendDesc : desc.BlendState.RenderTarget)
	{
		
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.BlendEnable));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.LogicOpEnable));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.SrcBlend));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.DestBlend));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.BlendOp));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.SrcBlendAlpha));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.DestBlendAlpha));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.BlendOpAlpha));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.LogicOp));
		serialisedDescription.append_range(FNV1::makeSpanView(blendDesc.RenderTargetWriteMask));
	}
	
	serialisedDescription.append_range(FNV1::makeSpanView(desc.SampleMask));

	//rasteriser state - tightly packed (all members 4 byte aligned) so can copy the whole thing.
	serialisedDescription.append_range(FNV1::makeSpanView(desc.RasterizerState));

	//depth stencil
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.DepthEnable));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.DepthWriteMask));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.DepthFunc));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.StencilEnable));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.StencilReadMask));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.StencilWriteMask));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.FrontFace));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DepthStencilState.BackFace));
	
	//Input Layout
	const auto inputDescriptions = std::span(desc.InputLayout.pInputElementDescs, desc.InputLayout.NumElements);
	for (const auto& inputSemantic : inputDescriptions)
	{
		auto stringView = std::string_view(inputSemantic.SemanticName);
		auto stringSpan = std::span<const std::byte>(std::bit_cast<const std::byte*>(stringView.data()), stringView.size());
		serialisedDescription.append_range(stringSpan); 
		
		serialisedDescription.append_range(FNV1::makeSpanView(inputSemantic.SemanticIndex));
		serialisedDescription.append_range(FNV1::makeSpanView(inputSemantic.Format));
		serialisedDescription.append_range(FNV1::makeSpanView(inputSemantic.InputSlot));
		serialisedDescription.append_range(FNV1::makeSpanView(inputSemantic.AlignedByteOffset));
		serialisedDescription.append_range(FNV1::makeSpanView(inputSemantic.InputSlotClass));
		serialisedDescription.append_range(FNV1::makeSpanView(inputSemantic.InstanceDataStepRate)); 
	}
	serialisedDescription.append_range(FNV1::makeSpanView(desc.InputLayout.NumElements)); 

	//THE REST
	serialisedDescription.append_range(FNV1::makeSpanView(desc.IBStripCutValue));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.PrimitiveTopologyType));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.NumRenderTargets));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.RTVFormats)); //this should work - sizeof(arr) == sizeinbytes!
	serialisedDescription.append_range(FNV1::makeSpanView(desc.DSVFormat));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.SampleDesc));
	serialisedDescription.append_range(FNV1::makeSpanView(desc.NodeMask));

	//skipping the cached pso - not going to hash it. 

	serialisedDescription.append_range(FNV1::makeSpanView(desc.Flags)); 


	this->hash = FNV1::Hash(serialisedDescription.data(), serialisedDescription.size()); 

}
