#include "MathHelpers.h"
#include <format>
#include <string>
#include <iostream>

void MathHelpers::PrintMatrix(const DirectX::SimpleMath::Matrix& Matrix)
{
	OutputDebugStringA(MatrixToString(Matrix).c_str());
}

std::string MathHelpers::MatrixToString(const DirectX::SimpleMath::Matrix& Matrix)
{
	DirectX::SimpleMath::Vector3 Translation = Matrix.Translation();
	DirectX::SimpleMath::Vector3 Rotation = Matrix.ToEuler();

	
	const auto string = std::format("|{0} {1} {2} {3}|\n", Matrix._11, Matrix._12, Matrix._13, Matrix._14) + 
		std::format("|{0} {1} {2} {3}|\n", Matrix._21, Matrix._22, Matrix._23, Matrix._24) + 
		std::format("|{0} {1} {2} {3}|\n", Matrix._31, Matrix._32, Matrix._33, Matrix._34) + 
		std::format("|{0} {1} {2} {3}|\n", Matrix._41, Matrix._42, Matrix._43, Matrix._44) + 
		std::format("Translation: [{0} {1} {2}]\n", Translation.x, Translation.y, Translation.z) +
		std::format("Rotation: [{0}, {1}, {2}]\n", Rotation.x, Rotation.y, Rotation.z);

	return string;
}

std::string MathHelpers::VectorToString(const DirectX::SimpleMath::Vector3& Vector)
{
	return std::format("[{0},{1},{2}]\n", Vector.x, Vector.y, Vector.z); 
}


