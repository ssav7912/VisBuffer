#pragma once
#include "DirectXMath.h"

class Camera
{
public:
	void Update();

	void SetRotation(DirectX::XMVECTOR Quat);
	void SetPosition(DirectX::XMFLOAT3 WorldPosition);	

	const DirectX::XMMATRIX& GetWorldToProjectionMatrix() const { return WorldToProjectionMatrix; };

private:

	DirectX::XMMATRIX CameraTransform; 

	DirectX::XMMATRIX WorldToViewMatrix;
	DirectX::XMMATRIX ViewToProjectionMatrix;
	DirectX::XMMATRIX WorldToProjectionMatrix;
};

