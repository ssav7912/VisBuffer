#pragma once
#define NOMINMAX 
#include <memory>
#include "stdafx.h"
//adapted from Microsoft MiniEngine sample. 

class Camera
{
public:

	//initialise matrices
	Camera() { UpdateProjectionMatrix(); Update(); };
	void Update();

	void SetRotation(DirectX::SimpleMath::Quaternion Quat);
	void SetPosition(DirectX::SimpleMath::Vector3 WorldPosition);
	void UpdateProjectionMatrix();

	void SetLookDirection(DirectX::SimpleMath::Vector3 Forward, DirectX::SimpleMath::Vector3 Up);
	void SetTransform(const DirectX::SimpleMath::Matrix& Transform);
	void SetFOV(float verticalFOVRadians) { VerticalFOVRadians = verticalFOVRadians; UpdateProjectionMatrix(); };
	void SetAspectRatio(float aspectRatio) { AspectRatio = aspectRatio; UpdateProjectionMatrix(); };
	const DirectX::SimpleMath::Matrix& GetTransform() const { return CameraTransform;  };
	const DirectX::SimpleMath::Matrix& GetWorldToProjectionMatrix() const { return WorldToProjectionMatrix; };

private:

	DirectX::SimpleMath::Matrix CameraTransform = DirectX::SimpleMath::Matrix::Identity; 

	DirectX::SimpleMath::Matrix WorldToViewMatrix = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix ViewToProjectionMatrix = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix WorldToProjectionMatrix = DirectX::SimpleMath::Matrix::Identity;

	float VerticalFOVRadians = DirectX::XMConvertToRadians(60.0f);
	float AspectRatio = 16.0f / 9.0f;
	float NearClipPlane = 0.001f;
	float FarClipPlane = 1000.0f;
	bool ReverseZ = true;
	bool InfiniteZ = false;
};

class CameraController
{
public:
	CameraController(void): ControlledCamera(std::make_shared<Camera>()) {};
	CameraController(std::shared_ptr<Camera> camera) : ControlledCamera(camera) {};

	//applied in world space
	void SetHeadingPitchAndPosition(float heading, float pitch, const DirectX::SimpleMath::Vector3& Position); 
	void GetHeadingPitchAndPosition(float& outHeading, float& outPitch, DirectX::SimpleMath::Vector3& outPosition) const;
	void OffsetHeadingPitchAndPosition(float heading, float pitch, const DirectX::SimpleMath::Vector3& Position);
	void ApplyPositionOffset(const DirectX::SimpleMath::Vector3& Position); 
	void ApplyRotationOffset(float headingOffset, float pitchOffset);

	//applied in local (i.e. camera/view) space. 
	void ApplyPositionOffsetLS(const DirectX::SimpleMath::Vector3& Offset);

	bool IsControlling() const { return ControlledCamera != nullptr; };
	const Camera& GetCamera() const { ASSERT(ControlledCamera != nullptr, "Camera must be registered!");  return *ControlledCamera.get(); };

	DirectX::SimpleMath::Matrix GetBasis() const { return DirectX::SimpleMath::Matrix(East, Up, North); };

private:

	std::shared_ptr<Camera> ControlledCamera;

	float CurrentHeading = 0.0;
	float CurrentPitch = 0.0;
	DirectX::SimpleMath::Vector3 CurrentPosition;

	//basis
	const DirectX::SimpleMath::Vector3 East = DirectX::SimpleMath::Vector3::Right;
	const DirectX::SimpleMath::Vector3 Up = DirectX::SimpleMath::Vector3::Up;
	const DirectX::SimpleMath::Vector3 North = -DirectX::SimpleMath::Vector3::Forward;
};