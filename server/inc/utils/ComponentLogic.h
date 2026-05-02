#ifndef COMPONENT_LOGIC_H
#define COMPONENT_LOGIC_H

#include "Config.h"

class ComponentLogic
{
  public:
	static std::map<UnitProperty, int> getUnitProperties(const std::map<std::string, unsigned int> &components);
	static std::string getInvalidConditionMessage(const std::map<UnitProperty, int> &properties,
												  const std::map<std::string, unsigned int> &componentCounts);
};

#endif // COMPONENT_LOGIC_H
