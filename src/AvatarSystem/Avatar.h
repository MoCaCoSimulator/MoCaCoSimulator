#pragma once

#include <vector>
#include <map>
#include <string>

#include "../AnimationCurve.h"
#include "../Customizable/CustomEnumerations.h"
#include "../SkinnedModel.h"
#include "AvatarJoint.h"

class Avatar
{
private:
    std::vector<Transform*> transforms;
    std::vector<AvatarJoint*> avatarJoints;
    SkinnedModel* skinnedModel;
public:
    Avatar(SkinnedModel* skinnedModel);
    ~Avatar();

    /// <summary>
    /// Calculates a anatomic angle
    /// </summary>
    /// <param name="jointType">The joint type you want to calculate a angle on</param>
    /// <param name="humanAnatomicAngle">The type of movement to calculate</param>
    /// <returns>The angle in degrees</returns>
    float GetAnatomicAngle(CustomEnums::AvatarJointType jointType, CustomEnums::HumanAnatomicAngleType humanAnatomicAngle);
    float CalculateCurrentAngle(AvatarJoint& avatarJoint, CustomEnums::AxisType rotationAxis);
    std::vector<float> CalculateAnatomicAnglesInOrder(AvatarJoint& avatarJoint, std::vector<AnatomicAngleInformation*> ordererAnatomicAngleInformations);
    AvatarJoint* GetAvatarJoint(CustomEnums::AvatarJointType humanJointType);
    std::vector<AvatarJoint*> GetAllAvatarJoints();
    std::vector<CustomEnums::HumanAnatomicAngleType> Avatar::GetAnatomicAngleTypesOfJoint(CustomEnums::AvatarJointType humanJointType);
    int GetJointCount();
private:
    void InitAvatarJoints();
};