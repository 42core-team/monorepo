#include "ComponentLogic.h"

std::map<UnitProperty, int> ComponentLogic::getUnitProperties(const std::map<std::string, unsigned int> &components)
{
	std::map<UnitProperty, int> properties;

	for (const auto &pair : components)
	{
		const std::string &componentId = pair.first;
		unsigned int count = pair.second;

		const ComponentConfig &config = Config::getComponentConfig(componentId);

		for (const auto &propertyPair : config.properties)
		{
			UnitProperty property = propertyPair.first;
			int value = propertyPair.second * count;

			properties[property] += value;
		}
	}

	return properties;
}
