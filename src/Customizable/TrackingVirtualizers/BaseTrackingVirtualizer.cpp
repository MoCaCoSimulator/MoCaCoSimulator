#include "BaseTrackingVirtualizer.h"

BaseTrackingVirtualizer::BaseTrackingVirtualizer(std::string name) : name(name) 
{ 
	AddParameter(new Parameter<std::string>("name", "unnamed"));
}

BaseTrackingVirtualizer::~BaseTrackingVirtualizer()
{
	for (std::map<std::string, BaseParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
		delete it->second;
	parameters.clear();
}

void BaseTrackingVirtualizer::AddParameter(BaseParameter* parameter)
{
	parameters[parameter->GetName()] = parameter;
}

std::vector<const BaseTrackingVirtualizer*>& BaseTrackingVirtualizer::registry()
{
    static std::vector<const BaseTrackingVirtualizer*> impl;
    return impl;
}

std::string BaseTrackingVirtualizer::GetName() const
{
    return name;
}
