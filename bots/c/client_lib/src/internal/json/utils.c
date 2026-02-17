// Clamp ulongs within range that won't result in scientific notation when encoded in JSON
unsigned long clamp_ulong_for_json(unsigned long value)
{
	const unsigned long JSON_SAFE_MAX = 999999UL;
	return (value > JSON_SAFE_MAX) ? JSON_SAFE_MAX : value;
}
