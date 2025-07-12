#pragma once
#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"
#include "DescriptorHeap.h"
class ImGuiHelper
{
public:
	inline static void Init(std::weak_ptr<DescriptorHeap> SRVHeap, HWND Hwnd, ID3D12Device* Device, ID3D12CommandQueue* CommandQueue, int32_t numFramesInFlight)
	{
		HeapManager = SRVHeap; 
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Hwnd);
		ImGui_ImplDX12_InitInfo Init{};
		Init.Device = Device;
		Init.CommandQueue = CommandQueue;
		Init.NumFramesInFlight = numFramesInFlight;
		Init.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		Init.SrvDescriptorHeap = SRVHeap.lock()->GetHeap();
		Init.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* init, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) -> void
			{
				ASSERT(!HeapManager.expired(), "ImGui attempted to allocate with a dead DescriptorHeap!");
				auto handle = HeapManager.lock()->Alloc();
				*outCpuHandle = static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(handle);
				*outGpuHandle = static_cast<D3D12_GPU_DESCRIPTOR_HANDLE>(handle);
				return;
			};

		Init.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* init, D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle) -> void
			{
				ASSERT(!HeapManager.expired(), "ImGui attempted to free with a dead DescriptorHeap!");
				DescriptorHandle handle{ CpuHandle, GpuHandle };
				HeapManager.lock()->Free(handle);
				return;
			};

		ImGui_ImplDX12_Init(&Init);
	}

	inline static void Destroy()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext(); 
		HeapManager.reset(); 
	
	}

private:

	//hold a ref to the SRV heap for lifetime purposes. 
	static inline std::weak_ptr<DescriptorHeap> HeapManager;

};