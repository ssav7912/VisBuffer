#include "Camera.h"
#include <algorithm>
#include "MathHelpers.h"


using namespace DirectX::SimpleMath;
void Camera::Update()
{
	WorldToViewMatrix = CameraTransform.Invert();
	WorldToProjectionMatrix = ViewToProjectionMatrix * WorldToViewMatrix; 

	OutputDebugStringA(MathHelpers::VectorToString(CameraTransform.Translation()).c_str());
	MathHelpers::PrintMatrix(CameraTransform); 
}

void Camera::SetRotation(Quaternion Quat)
{

	CameraTransform = Matrix::Transform(CameraTransform, Quat); 

}

void Camera::SetPosition(Vector3 WorldPosition)
{
	CameraTransform.Translation(WorldPosition); 
}

void Camera::UpdateProjectionMatrix()
{
	
	//TODO: infinite Z. 
	if (ReverseZ)
	{
		//ViewToProjectionMatrix = Matrix::CreateOrthographic(1, 1, NearClipPlane, FarClipPlane);
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

	//why does inverting right work here??
	//need to investigate that... TODO.
	Matrix basis{ right, Up, -Forward };
	basis.Translation(CameraTransform.Translation());
	CameraTransform = basis; 
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
	
	Matrix RotationY = Matrix::CreateRotationY(CurrentHeading);
	Matrix RotationX = Matrix::CreateRotationX(CurrentPitch);
	Matrix Basis = GetBasis(); 

	Matrix Orientation = Basis * RotationY * RotationX; 
	CurrentPosition = Position;
	Orientation.Translation(Position);
	ControlledCamera->SetTransform(Orientation);


	ControlledCamera->Update();

}

void CameraController::ApplyPositionOffset(const DirectX::SimpleMath::Vector3& Position)
{
	SetHeadingPitchAndPosition(CurrentHeading, CurrentPitch, CurrentPosition + Position); 
}

void CameraController::ApplyRotationOffset(float headingOffset, float pitchOffset)
{
	SetHeadingPitchAndPosition(CurrentHeading + headingOffset, CurrentPitch + pitchOffset, CurrentPosition); 
}

void CameraController::ApplyPositionOffsetLS(const DirectX::SimpleMath::Vector3& Offset)
{
	const auto Transformed = (this->GetCamera().GetTransform() * Matrix::CreateTranslation(Offset)).Translation();
	CurrentPosition = Transformed; 
	SetHeadingPitchAndPosition(CurrentHeading,CurrentPitch, Transformed); 
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
