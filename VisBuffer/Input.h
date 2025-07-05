#pragma once
#include "stdafx.h"
#include <GameInput.h>
#include <functional>


class Input
{
public:
	void Initialise();
	void Shutdown();

	void RegisterCallback(std::function<void(DirectX::SimpleMath::Vector2)> Callback);
	void UnregisterCallback() { CachedCallback = nullptr; };
	DirectX::SimpleMath::Vector2 GetMouseDelta() const { return PosDelta; };

private:
	static void CALLBACK ReadMouseEvent(GameInput::v1::GameInputCallbackToken token, void* context, GameInput::v1::IGameInputReading* Reading);

	//ONLY ONE PER PROCESS!
	static Microsoft::WRL::ComPtr<GameInput::v1::IGameInput> pGameInput;
	GameInput::v1::GameInputMouseState MouseState;
	std::function<void(DirectX::SimpleMath::Vector2)> CachedCallback;

	DirectX::SimpleMath::Vector2 LastMousePos {};
	DirectX::SimpleMath::Vector2 PosDelta{};
	GameInput::v1::GameInputCallbackToken ReadMouseEventHandle = -1;

};

