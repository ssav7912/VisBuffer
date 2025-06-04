#pragma once
#include <stdexcept>
#include <format>
#include "stdafx.h"

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	return std::format("HRESULT of {0:#08X}", static_cast<uint32_t>(hr));
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {};
	HRESULT Error() const { return m_hr; };
private:
	const HRESULT m_hr;
};

namespace ApplicationHelpers
{
	using Microsoft::WRL::ComPtr;

	inline void ThrowIfFailed(HRESULT result)
	{
		if (FAILED(result))
		{
			throw HrException(result);
		}
	}

}