#pragma once

#include "../tinygltf/tiny_gltf.h"
#include "../GeometryHeap.h"


class Model
{
public: 

	struct Section
	{
		BufferView Vertices;
		BufferView Colours;
		BufferView Indices; 
	};

	const std::vector<Section>& GetPrimitives() const { return Sections; };

	DirectX::SimpleMath::Matrix LocalToWorld;
	std::vector<Section> Sections;
	size_t GeometryHeapCBVIndex; 
	size_t GeometryHeapStartIndex;
};

