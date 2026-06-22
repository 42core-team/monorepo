#include "ComponentLogic.h"

#include "Config.h"

#include <stdexcept>

std::map<UnitProperty, int> ComponentLogic::getUnitProperties(const std::map<std::string, unsigned int> &components)
{
	std::map<UnitProperty, int> properties = Config::game().defaultUnitProperties;

	for (const auto &pair : components)
	{
		const std::string &componentId = pair.first;
		unsigned int count = pair.second;

		const ComponentConfig *config = Config::getComponentConfig(componentId);
		if (!config) continue;

		for (const auto &propertyPair : config.properties)
		{
			UnitProperty property = propertyPair.first;
			int value = propertyPair.second * count;

			properties[property] += value;
		}
	}

	return properties;
}

static int64_t evalInvalidConditionExpr(const json &expr, const std::map<UnitProperty, int> &properties,
										const std::map<std::string, unsigned int> &componentCounts)
{
	const std::string type = expr.at("type").get<std::string>();

	if (type == "const")
	{
		return expr.at("value").get<int64_t>();
	}

	if (type == "property")
	{
		const std::string propertyName = expr.at("property").get<std::string>();
		const UnitProperty property = Config::stringToUnitProperty(propertyName);

		auto it = properties.find(property);
		if (it == properties.end())
			throw std::runtime_error("Property not found in hypothetical unit: \"" + propertyName + "\".");

		return it->second;
	}

	if (type == "component_count")
	{
		const std::string componentId = expr.at("component").get<std::string>();

		auto it = componentCounts.find(componentId);
		if (it == componentCounts.end()) return 0;

		return static_cast<int64_t>(it->second);
	}

	if (type == "not")
	{
		const int64_t value = evalInvalidConditionExpr(expr.at("1"), properties, componentCounts);
		return value == 0 ? 1 : 0;
	}

	if (type == "ternary")
	{
		const int64_t condition = evalInvalidConditionExpr(expr.at("if"), properties, componentCounts);

		if (condition != 0) return evalInvalidConditionExpr(expr.at("then"), properties, componentCounts);

		return evalInvalidConditionExpr(expr.at("else"), properties, componentCounts);
	}

	const int64_t a = evalInvalidConditionExpr(expr.at("1"), properties, componentCounts);
	const int64_t b = evalInvalidConditionExpr(expr.at("2"), properties, componentCounts);

	if (type == "sum") return a + b;
	if (type == "subtract") return a - b;
	if (type == "multiply") return a * b;

	if (type == "divide")
	{
		if (b == 0) throw std::runtime_error("Division by zero in invalid condition expression.");

		return a / b;
	}

	if (type == "min") return std::min(a, b);
	if (type == "max") return std::max(a, b);

	if (type == "less_than") return a < b ? 1 : 0;
	if (type == "greater_than") return a > b ? 1 : 0;
	if (type == "less_than_or_equal") return a <= b ? 1 : 0;
	if (type == "greater_than_or_equal") return a >= b ? 1 : 0;
	if (type == "equal") return a == b ? 1 : 0;

	throw std::runtime_error("Unknown invalid condition expression type: \"" + type + "\".");
}

std::string ComponentLogic::getInvalidConditionMessage(const std::map<UnitProperty, int> &properties,
													   const std::map<std::string, unsigned int> &componentCounts)
{
	for (const InvalidConditionConfig &condition : Config::game().invalidConditions)
	{
		try
		{
			const int64_t result = evalInvalidConditionExpr(condition.condition, properties, componentCounts);

			if (result != 0) return condition.message;
		}
		catch (const std::exception &e)
		{
			return "invalid condition config: " + std::string(e.what());
		}
	}

	return "";
}
