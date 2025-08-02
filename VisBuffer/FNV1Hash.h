#pragma once
#include <cstdint>
#include <bit>
#include <span>
#include <cstddef>

namespace FNV1
{
	size_t Hash(const std::byte* buffer, size_t nbytes);

	//helper to return a uint8_t span for an arbitrary value reference
	//NOTE: this is going to include any padding bytes, if applicable!
	template<typename T>
	inline const std::span<const std::byte> makeSpanView(const T& data)
	{
		constexpr size_t dataSize = sizeof(data); 
		const std::byte* begin = std::bit_cast<const std::byte*>(&data); 

		return std::span(begin, dataSize); 
	}
}