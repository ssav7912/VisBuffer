#pragma once
#include "stdafx.h"
#include <string>

namespace MathHelpers
{

	void PrintMatrix(const DirectX::SimpleMath::Matrix& Matrix);

	std::string MatrixToString(const DirectX::SimpleMath::Matrix& Matrix);
	std::string VectorToString(const DirectX::SimpleMath::Vector3& Vector);
	std::string VectorToString(const DirectX::SimpleMath::Vector2& Vector);
};

