#include <random>
#include <cstdint>

std::string random_base32_seed()
{
	static constexpr char A[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
	uint64_t r = (static_cast<uint64_t>(std::random_device{}()) << 32) | std::random_device{}();
	std::string s; s.resize(13);
	for (int i = 0; i < 13; ++i) { s[i] = A[r & 31u]; r >>= 5; }
	return s;
}
