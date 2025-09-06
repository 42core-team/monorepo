#ifndef TRANSFER_GEMS_ACTION_H
#define TRANSFER_GEMS_ACTION_H

#include "Action.h"
#include "Common.h"
#include "json.hpp"

#include <vector>
using json = nlohmann::ordered_json;

class TransferGemsAction : public Action
{
  public:
	TransferGemsAction(json msg);

	std::string execute(Core *core);
	void decodeJSON(json msg);
	json encodeJSON();

  private:
	unsigned int source_id_;
	Position target_;
	unsigned int amount_;

	std::string dropGems(Core *core, Object *srcObj);
};

#endif // TRANSFER_GEMS_ACTION_H
