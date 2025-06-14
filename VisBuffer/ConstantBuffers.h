#pragma once
#include <directxmath.h>
#include "stdafx.h"

struct Vertex {
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};


struct alignas(256) MeshConstantBuffer
{
	DirectX::XMMATRIX LocalToWorld;
};

struct alignas(256) GlobalConstantBuffer
{
	DirectX::XMMATRIX ViewProjectionMatrix;
	DirectX::XMFLOAT3 CameraPosition; 
};
