#include "Camera.h"

void Camera::Update()
{
	WorldToViewMatrix = WorldToViewMatrix; //TODO

	WorldToProjectionMatrix = ViewToProjectionMatrix * WorldToViewMatrix; 
}

void Camera::SetRotation(DirectX::XMVECTOR Quat)
{

	DirectX::XMMatrixRotationQuaternion(Quat)* CameraTransform;

}

void Camera::SetPosition(DirectX::XMFLOAT3 WorldPosition)
{
}
