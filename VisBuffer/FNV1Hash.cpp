#include "FNV1Hash.h"

size_t FNV1::Hash(const std::byte* buffer, size_t nbytes)
{
	// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	//magic number for 64bit hash
	constexpr uint64_t offsetBasis = 0xcbf29ce484222325ul;
	constexpr uint64_t prime = 0x100000001b3ul;

	uint64_t hash = offsetBasis;
	for (size_t i = 0; i < nbytes; i++)
	{
		hash *= prime;
		hash ^= static_cast<uint8_t>(buffer[i]);
	}

	return hash;
}
