#include "AnatomicAngleInformation.h"

AnatomicAngleInformation::AnatomicAngleInformation
(
    CustomEnums::AxisType rotationAxis,
    CustomEnums::HumanAnatomicAngleType positiveAnatomicAngleType,
    CustomEnums::HumanAnatomicAngleType negativeAnatomicAngleType
)
    :
    positiveAnatomicAngleType(positiveAnatomicAngleType),
    negativeAnatomicAngleType(negativeAnatomicAngleType),
    rotationAxis(rotationAxis)
{
}

bool AnatomicAngleInformation::ContainsType(CustomEnums::HumanAnatomicAngleType humanAnatomicAngleType)
{
    return (positiveAnatomicAngleType == humanAnatomicAngleType || negativeAnatomicAngleType == humanAnatomicAngleType);
};