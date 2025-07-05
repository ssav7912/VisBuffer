#pragma once
#include "ConstantBuffers.h"
#include <array>
struct Cube
{
	const DirectX::XMFLOAT4 RED = { 1.0f, 0.0f, 0.0f, 0.0f };
	const DirectX::XMFLOAT4 GREEN = { 0.0f, 1.0f, 0.0f, 0.0f };
	const DirectX::XMFLOAT4 BLUE = { 0.0f, 0.0f, 1.0f, 0.0f };

	const std::array<Vertex, 36> Vertices{ {
			//tri 1 face front
			{{-1.0f, 1.0f, -1.0f}, RED},
			{{1.0f, -1.0f, -1.0f}, RED},
			{{-1.0f, -1.0f, -1.0f},RED},
			//tri2 face front 
			{{-1.0f, 1.0f, -1.0f}, RED },
			{{1.0f, 1.0f, -1.0f}, RED},
			{{-1.0f, -1.0f, -1.0f}, RED},

			//tri1 face right
			{{1.0f,1.0f,-1.0f}, GREEN},
			{{1.0f,-1.0f, 1.0f}, GREEN},
			{{1.0f,-1.0f, -1.0f}, GREEN},
			//tri2 face right
			{{1.0f, 1.0f, -1.0f}, GREEN},
			{{1.0f, 1.0f, 1.0f}, GREEN},
			{{1.0f, -1.0f, 1.0f}, GREEN},

			//tri1 face back
			{{1.0f, 1.0f, 1.0f},RED},
			{{1.0f,-1.0f,1.0f}, RED},
			{{-1.0f,-1.0f,1.0f}, RED},
			//tri2 face back
			{{-1.0f, -1.0f, 1.0f}, RED},
			{{-1.0f, 1.0f, 1.0f}, RED},
			{{1.0f, 1.0f, 1.0f}, RED},

			//tri1 face left
			{{-1.0f, -1.0f, 1.0f}, GREEN},
			{{-1.0f, 1.0f, -1.0f}, GREEN},
			{{-1.0f, 1.0f, 1.0f}, GREEN},
			//tri2 face left
			{{-1.0f, 1.0f, -1.0f}, GREEN},
			{{-1.0f, -1.0f, 1.0f}, GREEN},
			{{-1.0f,-1.0f, -1.0f}, GREEN},

			//tri1 top face
			{{-1.0f, 1.0f, -1.0f}, BLUE},
			{{-1.0f, 1.0f, 1.0f}, BLUE},
			{{1.0f, 1.0f, 1.0f}, BLUE},
			//tri2 top face
			{{1.0f, 1.0f, 1.0f}, BLUE},
			{{1.0f, 1.0f, -1.0f}, BLUE},
			{{-1.0f, 1.0f, -1.0f}, BLUE},

			//tri1 bottom face
			{{-1.0f,-1.0f,-1.0f}, BLUE},
			{{-1.0f,-1.0f,1.0f}, BLUE},
			{{1.0f, -1.0f, 1.0f}, BLUE},
			//tri2 bottom face
			{{1.0f, -1.0f, 1.0f}, BLUE},
			{{1.0f,-1.0f,-1.0f}, BLUE},
			{{-1.0f,-1.0f,-1.0f}, BLUE}
		} };
};