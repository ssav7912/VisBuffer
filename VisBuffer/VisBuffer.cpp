#include "VisBuffer.h"
#include "Win32Application.h"
#include "ApplicationHelpers.h"

using namespace ApplicationHelpers; 

VisBuffer::VisBuffer(uint32_t width, uint32_t height, std::wstring name) : DXApplication(width, height, name), 
	viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	scissorRect(0,0, static_cast<LONG>(width), static_cast<LONG>(height))
	
{


}

void VisBuffer::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void VisBuffer::OnUpdate()
{

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

	ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device)));

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

	ThrowIfFailed(swapChain.As(&localSwapChain));
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
		}
	}

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator))); 
}

void VisBuffer::LoadAssets()
{
}

void VisBuffer::PopulateCommandList()
{
	//TODO: fence guard, delegate?? 
	ThrowIfFailed(commandAllocator->Reset());
	
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));

	commandList->SetGraphicsRootSignature(RootSignature.Get());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect); 

	//back buffer will be a render target 
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)); 

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize); 
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr); 

	//record commands
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f }; 
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0); 

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)); 

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
