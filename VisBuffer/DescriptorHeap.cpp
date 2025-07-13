#include "DescriptorHeap.h"
#include "ApplicationHelpers.h"
#include "Assertions.h"
#include <ranges>
#include <optional>

void DescriptorHeap::Create(const std::wstring& HeapName, D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t MaxDescriptors)
{
	HeapDescription.Type = Type;
	HeapDescription.NumDescriptors = MaxDescriptors;
	HeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HeapDescription.NodeMask = 1;

	ApplicationHelpers::ThrowIfFailed(Device->CreateDescriptorHeap(&HeapDescription, IID_PPV_ARGS(Heap.ReleaseAndGetAddressOf())));
	Heap->SetName(HeapName.c_str());

	DescriptorSize = Device->GetDescriptorHandleIncrementSize(HeapDescription.Type);
	NumFreeDescriptors = HeapDescription.NumDescriptors;
	FirstHandle = DescriptorHandle(Heap->GetCPUDescriptorHandleForHeapStart(), Heap->GetGPUDescriptorHandleForHeapStart());
	
	FreeIndices.reserve(MaxDescriptors);
	FreeIndices.push_back({ 0u, MaxDescriptors }); 
		
}

DescriptorHandle DescriptorHeap::Alloc(uint32_t Count)
{
	ASSERT(HasSpace(Count), "Descriptor Heap is out of space. Increase the Heap size");

	/*
	Walk the free list until we find a block large enough to fit all requested descriptors.
	...average performance will degrade as memory allocations get more fragmented, worst case linear scan? 
	*/
	DescriptorHandle ReturnedHandle;
	std::optional<size_t> CompactionIndex; 
	for (size_t i = 0; i < FreeIndices.size(); i++)
	{
		BlockIndex& FreeBlock = FreeIndices[i]; 

		ASSERT(FreeBlock.numDescriptors > 0u, "Block allocation width of 0 is not allowed.");
		if (FreeBlock.numDescriptors >= Count)
		{
			//split the block in two.
			BlockIndex Left = {.offset = FreeBlock.offset, .numDescriptors = Count};
			BlockIndex Right = { .offset = Left.offset + Left.numDescriptors, .numDescriptors = FreeBlock.numDescriptors - Count }; 
		
			FreeBlock = Right; 


			//transform to byte addressed units rather than word (DescriptorSize) units
			const uint32_t ptrOffset = Left.offset * DescriptorSize;

			ReturnedHandle = FirstHandle + ptrOffset;
			ReturnedHandle.numDescriptors = Count; 

			//mark block for compaction/removal
			if (Right.numDescriptors == 0)
			{
				ASSERT(!CompactionIndex.has_value(), "Should not need to compact more than 1 block per allocation!");
				CompactionIndex = i; //flip from reverse index to forward index
			}
		}
	}

	if (CompactionIndex.has_value())
	{
		//god this API sucks
		FreeIndices.erase(FreeIndices.cbegin() + CompactionIndex.value());
	}
	NumFreeDescriptors -= Count;
	return ReturnedHandle;
}

void DescriptorHeap::Free(DescriptorHandle& Handle)
{
	ASSERT(ValidateHandle(Handle), "Not a valid handle for this descriptor heap");

	const BlockIndex newFreeBlock = { (Handle.GetCPUHandlePtr() - FirstHandle.GetCPUHandlePtr()) / this->DescriptorSize, Handle.numDescriptors};
	const size_t NewBlockSize =  newFreeBlock.numDescriptors; 



	//look for potential compaction candidate
	std::optional<size_t> InsertionIndex; 
	for (size_t i = 0; i < FreeIndices.size(); i++)
	{
		
		BlockIndex& freeBlock = FreeIndices[i]; 
		//early-outs: we can extend an existing free block with this free block. 
		//if the freed block bumps up on the left hand side of an existing freed block
		if (freeBlock.offset == newFreeBlock.offset + (newFreeBlock.numDescriptors))
		{
			freeBlock.offset = newFreeBlock.offset; 
			freeBlock.numDescriptors += newFreeBlock.numDescriptors; 
		}

		//or on the right side
		else if (newFreeBlock.offset == freeBlock.offset + (freeBlock.numDescriptors))
		{
			freeBlock.numDescriptors += newFreeBlock.numDescriptors; 
		}
		//otherwise, can do no compaction and need to insert in order in free list. 
		else {

			//deal with front/last edge cases
			if (i == 0)
			{
				const BlockIndex& neighbour = FreeIndices[i]; 
				//closest neighbour has at least 1 descriptor allocated between us
				if (neighbour.offset > newFreeBlock.offset + NewBlockSize)
				{
					InsertionIndex = 0;
					break;
				}
			}
			else if (i == FreeIndices.size() - 1)
			{
				const BlockIndex& neighbour = FreeIndices[i - 1];
				const size_t neighbourSize = (neighbour.numDescriptors);
				
				if (neighbour.offset + neighbourSize < newFreeBlock.offset)
				{
					InsertionIndex = FreeIndices.size() - 1; //pushback
					break;
				}
			}
			//somewhere in the middle
			else {
				const BlockIndex& leftNeighbour = FreeIndices[i];
				const BlockIndex& rightNeighbour = FreeIndices[i + 1];
				
				const uint32_t leftNeighbourSize = leftNeighbour.numDescriptors;

				const bool afterLeftNeighbour = (leftNeighbour.offset + leftNeighbourSize) < newFreeBlock.offset;
				const bool beforeRightNeighbour = rightNeighbour.offset > (newFreeBlock.offset + NewBlockSize);

				if (afterLeftNeighbour && beforeRightNeighbour)
				{
					ASSERT(!InsertionIndex.has_value(), "Attempting to insert a freed block more than once!");
					InsertionIndex = i + 1; //insert() inserts before the selected element, whereas i calculated for after...
					break; 
				}
			}
		}
	}
	//couldn't compact with an existing block, perform an insert
	if (InsertionIndex.has_value())
	{
		FreeIndices.insert(FreeIndices.begin() + InsertionIndex.value(), newFreeBlock);
	}
	//nothing free, just push a new block in. 
	else if (FreeIndices.empty())
	{
		FreeIndices.push_back(newFreeBlock);
	}

	NumFreeDescriptors += Handle.numDescriptors; 
	//null out the handle
	Handle = DescriptorHandle({ D3D12_GPU_VIRTUAL_ADDRESS_NULL }, { D3D12_GPU_VIRTUAL_ADDRESS_NULL }, 0u);
	return;


}

bool DescriptorHeap::ValidateHandle(const DescriptorHandle& Handle) const
{
	if (Handle.GetCPUHandlePtr() < FirstHandle.GetCPUHandlePtr() || Handle.GetCPUHandlePtr() >= FirstHandle.GetCPUHandlePtr() + HeapDescription.NumDescriptors * DescriptorSize)
	{
		return false;
	}

	if (Handle.GetGpuHandlePtr() - FirstHandle.GetGpuHandlePtr() != Handle.GetCPUHandlePtr() - FirstHandle.GetCPUHandlePtr())
	{
		return false;
	}

	return true;
}
