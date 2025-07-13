#include "VisBuffer.h"
#include "Win32Application.h"
#include "ApplicationHelpers.h"
#include "Cube.h"
#include "MathHelpers.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "ImGuiHelper.hpp"

using namespace ApplicationHelpers; 

VisBuffer::VisBuffer(uint32_t width, uint32_t height, std::wstring name) : DXApplication(width, height, name), 
	viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	scissorRect(0,0, static_cast<LONG>(width), static_cast<LONG>(height))
	
{
	

}

void VisBuffer::OnMouseEvent(DirectX::SimpleMath::Vector2 MouseXY)
{
	OutputDebugStringA(MathHelpers::VectorToString(MouseXY).c_str());
	Camera.ApplyRotationOffset(MouseXY.x * Camera.CameraSensitivity, MouseXY.y * Camera.CameraSensitivity); 
}

void VisBuffer::OnKeyDown(uint8_t key)
{
	float Delta = 0.01f;

	switch (key)
	{
	case 'W': { Camera.ApplyPositionOffsetLS(-DirectX::SimpleMath::Vector3::Forward * Camera.CameraSpeed); break; };
	case 'S': { Camera.ApplyPositionOffsetLS(DirectX::SimpleMath::Vector3::Forward * Camera.CameraSpeed); break; };
	case 'A': { Camera.ApplyPositionOffsetLS(DirectX::SimpleMath::Vector3::Left * Camera.CameraSpeed); break; };
	case 'D': { Camera.ApplyPositionOffsetLS(DirectX::SimpleMath::Vector3::Right * Camera.CameraSpeed); break; };
	case 'Q': { Camera.ApplyPositionOffsetLS(DirectX::SimpleMath::Vector3::Up * Camera.CameraSpeed); break; };
	case 'E': { Camera.ApplyPositionOffsetLS(DirectX::SimpleMath::Vector3::Down * Camera.CameraSpeed); break; };
	case VK_LEFT: { Camera.ApplyRotationOffset(-1.0f * Camera.CameraSpeed, 0.0f); break; };
	case VK_RIGHT:{ Camera.ApplyRotationOffset(1.0f * Camera.CameraSpeed, 0.0f); break; };
	case VK_UP: { Camera.ApplyRotationOffset(0.0f, 1.0f * Camera.CameraSpeed); break; };
	case VK_DOWN: { Camera.ApplyRotationOffset(0.0f, -1.0f * Camera.CameraSpeed); break; };
	}

}

void VisBuffer::OnMouseWheel(int16_t wheelDelta)
{
	Camera.CameraSpeed += static_cast<float>(wheelDelta) * 0.001f;

}

void VisBuffer::OnInit()
{
	DXApplication::OnInit(); 
	InputHandler.RegisterCallback({ [this](DirectX::SimpleMath::Vector2 MouseXY) { OnMouseEvent(MouseXY); } });
	LoadPipeline();
	LoadAssets();
	ImGuiHelper::Init(ShaderDescriptors, Win32Application::GetHwnd(), device.Get(), commandQueue.Get(), FrameCount);
}

void VisBuffer::OnUpdate()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	float _discard = 0.0f;
	DirectX::SimpleMath::Vector3 Position;
	Camera.GetHeadingPitchAndPosition(_discard, _discard, Position);
	GlobalConstants.CameraPosition = Position;
	GlobalConstants.ViewProjectionMatrix = Camera.GetCamera().GetWorldToProjectionMatrix(); //transpose for HLSL?
}

void VisBuffer::OnRender()
{
	PopulateCommandList();



	ID3D12CommandList* CommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists); 

	ThrowIfFailed(swapChain->Present(1, 0));

	MoveToNextFrame(); 
}

void VisBuffer::OnDestroy()
{
	WaitForGpu();

	CloseHandle(fenceEvent);

	ImGuiHelper::Destroy(); 

	DXApplication::OnDestroy(); 
}

void VisBuffer::LoadPipeline()
{
	uint32_t dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> DebugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			DebugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG; 
		}
	}
#endif
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))); 

	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(factory.Get(), hardwareAdapter.GetAddressOf());

	ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));

	//create resource objects, initialisation is done later.
	constexpr size_t MaxMeshes = 16;
	ShaderDescriptors = std::make_shared<DescriptorHeap>(device); 
	ShaderDescriptors->Create(L"Shader Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MaxMeshes);
	Geometry = std::make_unique<GeometryHeap>(device, ShaderDescriptors);

	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> localSwapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue.Get(), Win32Application::GetHwnd(), &swapChainDesc, nullptr, nullptr, &localSwapChain));

	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER)); 

	ThrowIfFailed(localSwapChain.As(&swapChain));
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	//descriptor heaps
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap))); 

		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); 
	}

	//frame resources
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart()); 
		for (uint32_t n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n]))); 
			device->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle); 
			rtvHandle.Offset(1, rtvDescriptorSize); 
			ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[n])));

			GlobalConstantResource[n] = std::make_unique<UploadBuffer>(device);
			GlobalConstantResource[n]->Create(std::format(L"Constant Buffer for frame {0}", n), sizeof(GlobalConstantBuffer));
			
		}
	
		
	}

	//geometry heap init
	Geometry->Create(L"Geometry Heap", MaxMeshes); 
}

void VisBuffer::LoadAssets()
{

	//TODO: Move into a root signature class
	{
		constexpr size_t NUMROOTCONSTANTS = 2;
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

		CD3DX12_ROOT_PARAMETER1 RootConstants[NUMROOTCONSTANTS] = {};
		RootConstants[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE);
		RootConstants[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE); 

		rootSignatureDesc.Init_1_1(NUMROOTCONSTANTS, RootConstants, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
		ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}

	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined (_DEBUG)
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0; 
#endif

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"../VisBuffer/shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"../VisBuffer/shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODescription = {};
		PSODescription.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		PSODescription.pRootSignature = RootSignature.Get(); 
		PSODescription.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		PSODescription.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		PSODescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		PSODescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PSODescription.DepthStencilState.DepthEnable = false;
		PSODescription.DepthStencilState.StencilEnable = false;
		PSODescription.SampleMask = UINT_MAX;
		PSODescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PSODescription.NumRenderTargets = 1;
		PSODescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		PSODescription.SampleDesc.Count = 1;
		ThrowIfFailed(device->CreateGraphicsPipelineState(&PSODescription, IID_PPV_ARGS(&pipelineState))); 

	}

	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[frameIndex].Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));
	
	{
		MeshConstantBuffer Mesh1 = {};
		DirectX::XMStoreFloat4x4(&Mesh1.LocalToWorld, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
		std::vector<Vertex> vertices{
			{ { 0.0f, 0.25f, -5.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f, -5.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f , -5.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};

		MeshConstantBuffer Mesh2 = {};
		DirectX::XMStoreFloat4x4(&Mesh2.LocalToWorld, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
		std::vector<Vertex> vertices2{
			{ { 0.0f, 0.75f , -5.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { 0.75f, -0.75f, -5.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.75f, -0.75f, -5.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } }
		};
		
		MeshConstantBuffer Mesh3 = {}; 
		DirectX::XMStoreFloat4x4(&Mesh3.LocalToWorld, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
		std::vector<Vertex> vertices3{
			{ { 0.0f, 5.0f, -5.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 1.0f, -5.0f, -5.0f }, { 1.0f, 0.0, 0.0f, 1.0f } },
			{ { -5.0f, -5.0f, -5.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }
		};


		Cube cube{};
		MeshConstantBuffer CubeCB = {};
		CubeCB.LocalToWorld = DirectX::SimpleMath::Matrix::CreateTranslation({ 1,1,-5});
		Geometry->BeginAddMesh(std::vector(cube.Vertices.begin(), cube.Vertices.end()), CubeCB, commandList.Get());


		Geometry->BeginAddMesh(vertices, Mesh1, commandList.Get());
		Geometry->BeginAddMesh(vertices, Mesh2, commandList.Get());
		Geometry->BeginAddMesh(vertices, Mesh3, commandList.Get());


		ThrowIfFailed(commandList->Close());

		ID3D12CommandList* copyList[] = { commandList.Get() };

		commandQueue->ExecuteCommandLists(1, copyList);
	}


	//crete synchronisation objects
	{
		ThrowIfFailed(device->CreateFence(fenceValues[frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

		fenceValues[frameIndex]++;

		fenceEvent = CreateEvent(nullptr, false, false, nullptr);
		if (fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}



	}

	WaitForGpu(); //wait for the copy to complete before moving to rendering...
	Geometry->EndAddMesh(); //clear upload heaps. 
}

void VisBuffer::PopulateCommandList()
{
	//TODO: fence guard, delegate?? 
	ThrowIfFailed(commandAllocators[frameIndex]->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocators[frameIndex].Get(), pipelineState.Get()));

	//TODO: put UI into another pass/cmndlist. Kind of inefficient to have to set pipeline state twice. 
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
	commandList->SetPipelineState(pipelineState.Get());

	commandList->SetGraphicsRootSignature(RootSignature.Get());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect); 

	//back buffer will be a render target 
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier); 

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize); 
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr); 

	//record commands
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f }; 
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GlobalConstantBuffer* GlobalConstantAddr = static_cast<GlobalConstantBuffer*>(GlobalConstantResource[frameIndex]->Map());
	std::memcpy(GlobalConstantAddr, &GlobalConstants, sizeof(GlobalConstantBuffer));
	GlobalConstantResource[frameIndex]->Unmap(); 


	commandList->SetGraphicsRootConstantBufferView(0, GlobalConstantResource[frameIndex]->GetGpuVirtualAddress());

	ASSERT(Geometry->GetMeshData().size() == Geometry->GetVertexBuffers().size(), "Num elements in Constant buffer and vertex buffer do not match!");
	for (size_t i = 0; i < Geometry->GetMeshData().size(); i++)
	{
		const auto& VBV = Geometry->GetVertexBuffers()[i];
		const D3D12_GPU_VIRTUAL_ADDRESS CBVOffset = Geometry->GetMeshData()[i].ConstantBufferOffset;
		const D3D12_GPU_VIRTUAL_ADDRESS CBAddress = Geometry->GetMeshConstants().GetGpuVirtualAddress() + CBVOffset;
		const uint32_t NumVertices = Geometry->GetMeshData()[i].NumVertices;


		commandList->SetGraphicsRootConstantBufferView(1, CBAddress);
		commandList->IASetVertexBuffers(0, 1, &VBV);
		commandList->DrawInstanced(NumVertices, 1, 0, 0);
	}
	
	auto backBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &backBarrier); 

	ThrowIfFailed(commandList->Close()); 
}


void VisBuffer::WaitForGpu()
{
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValues[frameIndex]));

	ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent)); 
	WaitForSingleObjectEx(fenceEvent, INFINITE, false); 

	fenceValues[frameIndex]++; 

}

void VisBuffer::MoveToNextFrame()
{
	const uint64_t currentFenceValue = fenceValues[frameIndex];
	ThrowIfFailed(commandQueue->Signal(fence.Get(), currentFenceValue));

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	if (fence->GetCompletedValue() < fenceValues[frameIndex])
	{
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[frameIndex], fenceEvent));
		WaitForSingleObjectEx(fenceEvent, INFINITE, false); 
	}

	fenceValues[frameIndex] = currentFenceValue + 1; 
}
