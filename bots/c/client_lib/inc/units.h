#ifndef UNITS_H
#define UNITS_H

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief Type of unit.
typedef enum e_unit_type
{
	UNIT_WARRIOR = 0,
	UNIT_MINER = 1,
	UNIT_CARRIER = 2,
	UNIT_TANK = 3
} t_unit_type;

#ifdef __cplusplus
}
#endif

#endif // UNITS_H
