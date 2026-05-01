#ifndef COMPONENT_LOGIC_H
#define COMPONENT_LOGIC_H

#include "Config.h"

class ComponentLogic
{
  public:
	static std::map<UnitProperty, int> getUnitProperties(const std::map<std::string, unsigned int> &components);
};

#endif // COMPONENT_LOGIC_H
