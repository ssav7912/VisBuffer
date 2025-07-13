#include "pch.h"
#include "CppUnitTest.h"
#include "../VisBuffer/Camera.h"
#include "../VisBuffer/stdafx.h"
#include <format>
#include "../VisBuffer/MathHelpers.h"
#include "../VisBuffer/DescriptorHeap.h"
#include "../VisBuffer/ApplicationHelpers.h"
#include "../VisBuffer/DXApplication.h"
using namespace ApplicationHelpers;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace DirectX::SimpleMath;

namespace Tests
{
	TEST_CLASS(DescriptorHeapTests)
	{

		constexpr static uint32_t DescriptorSize = 32;
		static inline std::shared_ptr<DescriptorHeap> Descriptors;
		static inline Microsoft::WRL::ComPtr<ID3D12Device4> device;

	public:
		TEST_CLASS_INITIALIZE(Init)
		{

			uint32_t dxgiFactoryFlags = 0;

			{
				ComPtr<ID3D12Debug> DebugController;
				if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
				{
					DebugController->EnableDebugLayer();
					dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
				}
			}
			ComPtr<IDXGIFactory4> factory;
			ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

			ComPtr<IDXGIAdapter1> hardwareAdapter;
			DXApplication::GetHardwareAdapter(factory.Get(), hardwareAdapter.GetAddressOf());

			ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));

		}

		TEST_METHOD_INITIALIZE(HeapReset)
		{
			Descriptors = std::make_shared<DescriptorHeap>(device.Get());
			Descriptors->Create(L"Unit Test Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DescriptorSize);
		}

		TEST_METHOD(DescriptorHeapAlloc)
		{

			auto handle = Descriptors->Alloc();
			Assert::IsFalse(handle.isNull());
			Assert::IsTrue(Descriptors->HasSpace(DescriptorSize - 1));

			Descriptors->Free(handle);
			Assert::IsTrue(handle.isNull());
			Assert::IsTrue(Descriptors->HasSpace(32));

		};

		TEST_METHOD(DescriptorHeapFragmentation)
		{

			auto handle1 = Descriptors->Alloc();			
			auto handle1Offset = Descriptors->GetHandleOffset(handle1);
			
			auto handle2 = Descriptors->Alloc();
			auto handle2Offset = Descriptors->GetHandleOffset(handle2); 
			
			auto handle3 = Descriptors->Alloc();

			
			Assert::IsTrue(Descriptors->HasSpace(DescriptorSize - 3));
			//handle 2 is allocated adjacent to handle 1
			Assert::AreEqual(handle1Offset + 1, handle2Offset);

			Descriptors->Free(handle1); 
			Assert::IsTrue(Descriptors->HasSpace(DescriptorSize - 2));
			
			//hmm how do i test compaction??
			auto bigHandle = Descriptors->Alloc(5);
			
			//check existing allocations don't overlap with new one
			bool isLeftOfBigHandle = handle2.GetCPUHandlePtr() < bigHandle.GetCPUHandlePtr();
			bool isRightOfBigHandle = handle2.GetCPUHandlePtr() > bigHandle.GetCPUHandlePtr() + (5 * Descriptors->GetDescriptorSize());

			bool isOverlappingHandle = !isLeftOfBigHandle && !isRightOfBigHandle;

			Assert::IsFalse(isOverlappingHandle); 

		}

		TEST_METHOD(DescriptorHeapCompaction)
		{
			//make a smaller heap
			Descriptors->Destroy();
			Descriptors->Create(L"Unit Test Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 3);
		
		
			auto handle1 = Descriptors->Alloc();
			size_t handle1Offset = handle1.GetCPUHandlePtr(); 
			auto handle2 = Descriptors->Alloc();
			size_t handle2Offset = handle2.GetCPUHandlePtr(); 
			auto handle3 = Descriptors->Alloc();

			//freeing first block and reallocating places it at the start of the heap
			//(only empty space)
			Descriptors->Free(handle1);
			handle1 = Descriptors->Alloc();
			Assert::AreEqual(handle1Offset, handle1.GetCPUHandlePtr()); 

			//new allocation gets placed at start of 1st index.
			//this proves compaction is working, otherwise first free block (from back) would be at index 2. 
			//sort of testing side effects here... may be easier if i split the free list tracker out of the DescriptorHeap interface. 
			Descriptors->Free(handle3);
			Descriptors->Free(handle2);
			handle2 = Descriptors->Alloc();
			Assert::AreEqual(handle2Offset, handle2.GetCPUHandlePtr()); 

		}

		TEST_METHOD_CLEANUP(Cleanup)
		{
			Descriptors->Destroy(); 
		}

		TEST_CLASS_CLEANUP(ClassDestroy)
		{
			Descriptors.reset();
			device.Reset();
		}
	};

}