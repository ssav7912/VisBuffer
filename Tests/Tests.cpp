#include "pch.h"
#include "CppUnitTest.h"
#include "../VisBuffer/Camera.h"
#include "../VisBuffer/stdafx.h"
#include <format>
#include "../VisBuffer/MathHelpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace DirectX::SimpleMath;

//specialisations for types
namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			template<> static std::wstring ToString<Matrix>(const Matrix& t) { RETURN_WIDE_STRING(MathHelpers::MatrixToString(t).c_str()); };
			template<> static std::wstring ToString<Vector3>(const Vector3& t) { RETURN_WIDE_STRING(MathHelpers::VectorToString(t).c_str()); };
			template<> static void Assert::AreEqual<Vector3>(const Vector3& expected, const Vector3& actual, const wchar_t* message, const __LineInfo* pLineInfo)
			{
				constexpr float tolerance = 1e-05f; 
				Vector3 diff = expected - actual; 
				bool condition = fabs(diff.x) < tolerance && fabs(diff.y) < tolerance && fabs(diff.z) < tolerance;
				FailOnCondition(condition, EQUALS_MESSAGE(expected, actual, message), pLineInfo); 
			}
		}
	}
}
namespace Tests
{
	TEST_CLASS(CameraMatrixTests)
	{
	public:
		TEST_METHOD(CameraTranslationTests)
		{
			CameraController Camera{};

			auto Transform = Camera.GetCamera().GetTransform();
			Assert::AreEqual(Matrix::Identity, Transform);
			

			Camera.ApplyPositionOffset({ 1,0,0 });

			auto NewTransform = Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,0,0 }, NewTransform.Translation()); 

			Camera.ApplyPositionOffset({ 0,0,1 });
			NewTransform = Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,0,1 }, NewTransform.Translation());

			Camera.ApplyPositionOffset({ 0,1,0 });
			NewTransform = Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,1,1 }, NewTransform.Translation());
			

			Camera.SetHeadingPitchAndPosition(0, 0, Vector3::Zero); 
			
			Camera.ApplyPositionOffsetLS({ 1,0,0 });
			NewTransform = Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,0,0 }, NewTransform.Translation());

			Camera.ApplyPositionOffsetLS({ 0,0,1 });
			NewTransform = Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,0,1 }, NewTransform.Translation());

			Camera.ApplyPositionOffsetLS({ 0,1,0 });
			NewTransform = Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,1,1 }, NewTransform.Translation());
		}

		
		TEST_METHOD(CameraRotationTests)
		{
			CameraController Camera{};

			auto Transform = Camera.GetCamera().GetTransform();
			Assert::AreEqual(Matrix::Identity, Transform);
			
			const float Pan90Right = DirectX::XMConvertToRadians(90);
			Camera.ApplyRotationOffset(Pan90Right, 0);

			auto NewTransform = Camera.GetCamera().GetTransform();
			Matrix ExpectedTransform = Camera.GetBasis() * Matrix::CreateFromAxisAngle({ 0,1,0 }, Pan90Right);
			Assert::AreEqual(ExpectedTransform.ToEuler(), NewTransform.ToEuler());

			const float Pan90Left = DirectX::XMConvertToRadians(-90);
			Camera.SetHeadingPitchAndPosition(Pan90Left, 0, { 0,0,0 });
			NewTransform = Camera.GetCamera().GetTransform();
			ExpectedTransform = Camera.GetBasis() * Matrix::CreateFromAxisAngle({ 0,1,0 }, Pan90Left);
			Assert::AreEqual(ExpectedTransform.ToEuler(), NewTransform.ToEuler()); 

			const float Pitch90Up = DirectX::XMConvertToRadians(90);
			Camera.SetHeadingPitchAndPosition(0, Pitch90Up, { 0,0,0 });
			NewTransform = Camera.GetCamera().GetTransform();
			ExpectedTransform = Camera.GetBasis() * Matrix::CreateFromAxisAngle({1,0,0 }, Pitch90Up);
			Assert::AreEqual(ExpectedTransform.ToEuler(), NewTransform.ToEuler()); 


			const float Pitch90Down = DirectX::XMConvertToRadians(-90);
			Camera.SetHeadingPitchAndPosition(0, Pitch90Down, { 0,0,0 });
			NewTransform = Camera.GetCamera().GetTransform();
			ExpectedTransform = Camera.GetBasis() * Matrix::CreateFromAxisAngle({ 1,0,0 }, Pitch90Down);
			Assert::AreEqual(ExpectedTransform.ToEuler(), NewTransform.ToEuler());


		}

		TEST_METHOD(CombinedTransformTests)
		{

			CameraController Camera{};

			auto Transform = Camera.GetCamera().GetTransform();
			Assert::AreEqual(Matrix::Identity, Transform);
			Assert::AreEqual(Matrix::Identity.ToEuler(), Transform.ToEuler());


			Camera.ApplyPositionOffset({ 1,0,0 });

			auto NewTransform = Camera.GetBasis() * Camera.GetCamera().GetTransform();
			Assert::AreEqual({ 1,0,0 }, NewTransform.Translation());
			Assert::AreEqual(Matrix::Identity.ToEuler(), NewTransform.ToEuler()); 
		}
	};
}
