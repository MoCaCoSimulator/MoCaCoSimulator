#include "BaseErrorMetric.h"
#include "../../Animator.h"

BaseErrorMetric::BaseErrorMetric(std::string name) : name(name)
{
    AddParameter(new Parameter<std::string>("Name", name));
}

std::vector<const BaseErrorMetric*>& BaseErrorMetric::registry()
{
    static std::vector<const BaseErrorMetric*> impl;
    return impl;
}

std::string BaseErrorMetric::GetName() const
{
    return name;
}

int BaseErrorMetric::GetSamplerate()
{
	return dynamic_cast<Parameter<int>*>(parameters["SampleRate"])->GetValue();
}

void BaseErrorMetric::AddParameter(BaseParameter* parameter)
{
    parameters[parameter->GetName()] = parameter;
}