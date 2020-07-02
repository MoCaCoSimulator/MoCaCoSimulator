#pragma once
#include "../MeshModel.h"
#include "AnatomicAngleInformation.h"
#include "../Transform.h"
#include "../Customizable/CustomEnumerations.h"

class AvatarJoint
{
public:
    CustomEnums::AvatarJointType humanJointType;
    Transform* transform;
    Quaternion startRotation;
public:
    AnatomicAngleInformation* twistAnatomicAngleInformation;
    AnatomicAngleInformation* firstDOFAnatomicAngleInformation;
    AnatomicAngleInformation* secondDOFAnatomicAngleInformation;

    AvatarJoint(CustomEnums::AvatarJointType humanJointType, Transform* transform);
    ~AvatarJoint();

    CustomEnums::AxisType GetForward();
    Vector3 GetAxis(CustomEnums::AxisType axisType, bool asStart);
};