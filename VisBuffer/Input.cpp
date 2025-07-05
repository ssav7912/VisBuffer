#include "Input.h"
#include "ApplicationHelpers.h"

using namespace GameInput::v1;
using namespace Microsoft::WRL;
using namespace DirectX;

ComPtr<IGameInput> Input::pGameInput = nullptr; 

void Input::Initialise()
{
	if (!pGameInput)
	{
		//TODO: users need to install GDK. Write an installer for this...
		ApplicationHelpers::ThrowIfFailed(GameInputCreate(&pGameInput));
		pGameInput->SetFocusPolicy(GameInputExclusiveForegroundInput); 
	}
	else {
		OutputDebugStringA("A GameInput device already exists, not initialising a new one.");
	}
	

	pGameInput->RegisterReadingCallback(nullptr, GameInputKindMouse, static_cast<void*>(this), &ReadMouseEvent, &ReadMouseEventHandle);
}

void Input::Shutdown()
{
	UnregisterCallback();
	if (pGameInput)
	{
		pGameInput->UnregisterCallback(this->ReadMouseEventHandle);
	}
}

void Input::RegisterCallback(std::function<void(DirectX::SimpleMath::Vector2)> Callback)
{
	this->CachedCallback = Callback; 

}

void CALLBACK Input::ReadMouseEvent(GameInputCallbackToken token, void* context, IGameInputReading* Reading)
{
	ASSERT(context, "Context was null!");
	auto thisPtr = static_cast<Input*>(context); 

	thisPtr->ReadMouseEventHandle = token;
	Reading->GetMouseState(&thisPtr->MouseState); 

	SimpleMath::Vector2 NewPos = SimpleMath::Vector2{ 
		static_cast<float>(thisPtr->MouseState.positionX), 
		static_cast<float>(thisPtr->MouseState.positionY) };
	
	thisPtr->PosDelta = thisPtr->LastMousePos - NewPos;
	thisPtr->LastMousePos = NewPos;
 
	if (thisPtr->CachedCallback)
	{
		thisPtr->CachedCallback(thisPtr->GetMouseDelta());
	}

}
