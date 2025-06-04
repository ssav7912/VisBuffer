#pragma once
#include "DXApplication.h"
#include <cstdint>
#include "GeometryHeap.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using std::uint32_t;
using std::uint64_t;


class VisBuffer : public DXApplication
{



public: 
    VisBuffer(uint32_t width, uint32_t height, std::wstring name);



    virtual void OnInit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;
    virtual void OnDestroy() override;

private:
    static constexpr ::uint32_t FrameCount = 2;

    CD3DX12_VIEWPORT viewport;
    CD3DX12_RECT scissorRect;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12Device4> device;
    ComPtr<ID3D12Resource> resource;
    ComPtr<ID3D12Resource> renderTargets[FrameCount]; 
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12DescriptorHeap> rtvHeap; 
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12GraphicsCommandList2> commandList;
    ::uint32_t rtvDescriptorSize = 0;

    //app resources - should be abstracted? 
    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    GeometryHeap Geometry;

    //sync objects
    ::uint32_t frameIndex = 0;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    uint64_t fenceValues[FrameCount];

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();

    void WaitForGpu();
    void MoveToNextFrame();


};

