#include "CreateAction.h"

CreateAction::CreateAction(json msg) : Action(ActionType::CREATE)
{
	if (!msg.contains("type_id"))
	{
		is_valid_ = false;
		return;
	}

	type_id_ = msg["type_id"];
}
