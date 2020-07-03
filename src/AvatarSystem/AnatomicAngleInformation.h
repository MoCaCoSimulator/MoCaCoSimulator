#pragma once

#include "../Customizable/CustomEnumerations.h"

class AnatomicAngleInformation
{
public:
    AnatomicAngleInformation(CustomEnums::AxisType rotationAxis, CustomEnums::HumanAnatomicAngleType positiveAnatomicAngleType, CustomEnums::HumanAnatomicAngleType negativeAnatomicAngleType);

    CustomEnums::HumanAnatomicAngleType positiveAnatomicAngleType;
    CustomEnums::HumanAnatomicAngleType negativeAnatomicAngleType;
    CustomEnums::AxisType rotationAxis;

    bool ContainsType(CustomEnums::HumanAnatomicAngleType humanAnatomicAngleType);
};