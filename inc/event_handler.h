#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "con_lib.h"

extern void * user_data;

typedef struct s_obj t_obj;
typedef struct s_pos t_pos;
typedef enum e_obj_state t_obj_state;

typedef struct s_event_handler {
	// Called on every global tick (how often per second can be found in config)
	void (*on_tick)(unsigned long tick, void *custom_data);
	// Called once when the game starts
	void (*on_start)(void *custom_data);
	// Called once when the game exits
	void (*on_exit)(void *custom_data);

	// Called once per object on each tick (in addition to the global tick)
	void (*on_object_ticked)(t_obj *obj, unsigned long tick, void *custom_data);
	// Called when an object's state changes
	void (*on_object_state_change)(t_obj *obj, t_obj_state old_state, t_obj_state new_state, void *custom_data);
	// Called when an object's position changes
	void (*on_object_pos_change)(t_obj *obj, t_pos old_pos, t_pos new_pos, void *custom_data);
	// Called when an object's balance changes
	void (*on_object_balance_change)(t_obj *obj, unsigned long old_balance, unsigned long new_balance, void *custom_data);
	// Called when an object's health changes
	void (*on_object_health_change)(t_obj *obj, unsigned long old_hp, unsigned long new_hp, void *custom_data);
} t_event_handler;

extern t_event_handler event_handler;

#endif // EVENT_HANDLER_H
