#include "Camera.h"
#include <algorithm>


using namespace DirectX::SimpleMath;
void Camera::Update()
{
	WorldToViewMatrix = CameraTransform.Invert();
	WorldToProjectionMatrix = ViewToProjectionMatrix * WorldToViewMatrix; 
}

void Camera::SetRotation(Quaternion Quat)
{

	CameraTransform = Matrix::Transform(CameraTransform, Quat); 

}

void Camera::SetPosition(Vector3 WorldPosition)
{
	CameraTransform *= Matrix::CreateTranslation(WorldPosition); 
}

void Camera::UpdateProjectionMatrix()
{
	
	//TODO: infinite Z. 
	if (ReverseZ)
	{
		ViewToProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(VerticalFOVRadians, AspectRatio, FarClipPlane, NearClipPlane);
	}
	else {
		ViewToProjectionMatrix = Matrix::CreatePerspectiveFieldOfView(VerticalFOVRadians, AspectRatio, NearClipPlane, FarClipPlane);
	}

}

void Camera::SetLookDirection(DirectX::SimpleMath::Vector3 Forward, DirectX::SimpleMath::Vector3 Up)
{
	float forwardLenSquared = Forward.LengthSquared();
	Forward = forwardLenSquared < 0.000001f ? -Vector3::UnitZ : Forward * (1.0f / std::sqrtf(forwardLenSquared));

	Vector3 right = Forward.Cross(Up);
	float rightLenSquare = right.LengthSquared();
	right = (rightLenSquare < 0.000001f) ? static_cast<Vector3>((Quaternion(Vector3::UnitY, -DirectX::XM_PIDIV2) * Forward)) : (right * (1.0f / std::sqrtf(rightLenSquare)));

	Up = right.Cross(Forward);

	Matrix basis{ right, Up, -Forward }; 
	SetRotation(Quaternion::CreateFromRotationMatrix(basis)); 
}

void Camera::SetTransform(const DirectX::SimpleMath::Matrix& Transform)
{
	SetLookDirection(Transform.Forward(), Transform.Up());
	SetPosition(Transform.Translation());

}

void CameraController::SetHeadingPitchAndPosition(float heading, float pitch, const DirectX::SimpleMath::Vector3& Position)
{
	ASSERT(ControlledCamera != nullptr, "Attempting to set heading on a null camera.");

	CurrentHeading = heading;
	if (CurrentHeading > DirectX::XM_PI)
	{
		CurrentHeading -= DirectX::XM_2PI;
	}
	else if (CurrentHeading <= -DirectX::XM_PI)
	{
		CurrentHeading += DirectX::XM_2PI;
	}

	CurrentPitch = pitch;
	CurrentPitch = std::min(DirectX::XM_PIDIV2, CurrentPitch);
	CurrentPitch = std::max(-DirectX::XM_PIDIV2, CurrentPitch);

	Matrix Orientation = Matrix(East, Up, -North) * Matrix::CreateRotationY(CurrentHeading) * Matrix::CreateRotationX(CurrentPitch); 
	Orientation.Translation(Position);

	ControlledCamera->SetTransform(Orientation);

	ControlledCamera->Update();

}

void CameraController::ApplyPositionOffset(const DirectX::SimpleMath::Vector3& Position)
{
	SetHeadingPitchAndPosition(CurrentHeading, CurrentPitch, CurrentPosition + Position); 
}

void CameraController::GetHeadingPitchAndPosition(float& outHeading, float& outPitch, DirectX::SimpleMath::Vector3& outPosition) const
{
	outHeading = CurrentHeading;
	outPitch = CurrentPitch;
	outPosition = CurrentPosition;
}

void CameraController::OffsetHeadingPitchAndPosition(float heading, float pitch, const DirectX::SimpleMath::Vector3& Position)
{
	SetHeadingPitchAndPosition(CurrentHeading + heading, CurrentPitch + pitch, CurrentPosition + Position); 
}
