#include "AvatarJoint.h"
#include "../Utils.h"
#include <QDebug>

AvatarJoint::AvatarJoint(CustomEnums::AvatarJointType humanJointType, Transform* transform) :
    humanJointType(humanJointType),
    transform(transform),
    firstDOFAnatomicAngleInformation(nullptr),
    secondDOFAnatomicAngleInformation(nullptr),
    twistAnatomicAngleInformation(nullptr)
{
    Matrix tempMat = transform->LocalMatrix();
    globalOffsetMatrix = tempMat.invert();
    //localOffsetMatrix = transform->LocalMatrix().invert();
    globalOffsetRotation = transform->GlobalMatrix().invert().rotation();

    qDebug() << "globalOffsetMatrix: " << globalOffsetMatrix.rotation().toString().c_str();
    qDebug() << "globalOffsetRotation: " << globalOffsetRotation.toString().c_str();

    startRotation = transform->LocalRotation();// *localOffsetMatrix.rotation();
}

AvatarJoint::~AvatarJoint()
{
    if (twistAnatomicAngleInformation)
        delete twistAnatomicAngleInformation;
    if (firstDOFAnatomicAngleInformation)
        delete firstDOFAnatomicAngleInformation;
    if (secondDOFAnatomicAngleInformation)
        delete secondDOFAnatomicAngleInformation;
}

CustomEnums::AxisType AvatarJoint::GetForward()
{
    return twistAnatomicAngleInformation->rotationAxis;
}

Vector3 AvatarJoint::GetAxis(CustomEnums::AxisType axisType, bool asStart)
{
    Vector3 axis = Vector3::zero;
    Quaternion currentRotation = transform->LocalRotation();

    if (asStart)
    {
        transform->LocalRotation(startRotation);

        if(humanJointType == CustomEnums::AvatarJointType::LeftShoulder
        || humanJointType == CustomEnums::AvatarJointType::RightShoulder)
        {
            AnatomicAngleInformation* info;
            bool positive = false;

            if (firstDOFAnatomicAngleInformation->positiveAnatomicAngleType == CustomEnums::HumanAnatomicAngleType::Abduktion)
            {
                info = firstDOFAnatomicAngleInformation;
                positive = true;
            }
            else if (firstDOFAnatomicAngleInformation->negativeAnatomicAngleType == CustomEnums::HumanAnatomicAngleType::Abduktion)
            {
                info = firstDOFAnatomicAngleInformation;
                positive = false;
            }
            else if (secondDOFAnatomicAngleInformation->positiveAnatomicAngleType == CustomEnums::HumanAnatomicAngleType::Abduktion)
            {
                info = secondDOFAnatomicAngleInformation;
                positive = true;
            }
            else if (secondDOFAnatomicAngleInformation->negativeAnatomicAngleType == CustomEnums::HumanAnatomicAngleType::Abduktion)
            {
                info = secondDOFAnatomicAngleInformation;
                positive = false;
            }
            else
            {
                info = secondDOFAnatomicAngleInformation;
                positive = false;
            }

            transform->LocalRotation(transform->LocalRotation() * globalOffsetMatrix.rotation());

            Vector3 abductionAxis = Vector3::zero;
            switch (info->rotationAxis)
            {
                case CustomEnums::AxisType::Forward: abductionAxis      = transform->Forward(); break;
                case CustomEnums::AxisType::MinusForward: abductionAxis = -transform->Forward(); break;
                case CustomEnums::AxisType::Up: abductionAxis           = transform->Up(); break;
                case CustomEnums::AxisType::MinusUp: abductionAxis      = -transform->Up(); break;
                case CustomEnums::AxisType::Right: abductionAxis        = transform->Right(); break;
                case CustomEnums::AxisType::MinusRight: abductionAxis   = -transform->Right(); break;
            }

            transform->LocalRotation(startRotation);

            float angle = positive ? -90 : 90;  
            transform->Rotate(abductionAxis, angle, Transform::Space::World);
        }
    }

    transform->LocalRotation(transform->LocalRotation() * globalOffsetMatrix.rotation());

    switch (axisType)
    {
        case CustomEnums::AxisType::Forward: axis = transform->Forward(); break;
        case CustomEnums::AxisType::MinusForward: axis = -transform->Forward(); break;
        case CustomEnums::AxisType::Up: axis = transform->Up(); break;
        case CustomEnums::AxisType::MinusUp: axis = -transform->Up(); break;
        case CustomEnums::AxisType::Right: axis = transform->Right(); break;
        case CustomEnums::AxisType::MinusRight: axis = -transform->Right(); break;
    }

    transform->LocalRotation(currentRotation);

    return axis;
}