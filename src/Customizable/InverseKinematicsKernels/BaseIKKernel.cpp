#include "BaseIKKernel.h"

BaseIKKernel::BaseIKKernel(std::string name) : name(name) { }

std::vector<BaseIKKernel*>& BaseIKKernel::registry()
{
    static std::vector<BaseIKKernel*> impl;
    return impl;
}

std::string BaseIKKernel::GetName() const
{
    return name;
}

void BaseIKKernel::AddParameter(BaseParameter* parameter)
{
    parameters[parameter->GetName()] = parameter;
}