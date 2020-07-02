#include "Avatar.h"

#include <QDebug>

Avatar::Avatar(SkinnedModel* skinnedModel) :
    skinnedModel(skinnedModel),
    transforms(skinnedModel->ConvertRepresentationToTransforms())
{
    InitAvatarJoints();
}

Avatar::~Avatar()
{
    for (AvatarJoint* joint : avatarJoints)
        delete joint;
    avatarJoints.clear();
}

CustomEnums::AxisType Avatar::GetAxis(Vector3 dir, Transform* transform)
{
    std::map<CustomEnums::AxisType, float> pairs;
    float forward = Vector3::AngleDegree(dir, transform->Forward());
    pairs[CustomEnums::AxisType::Forward] = forward;
    float minusForward = Vector3::AngleDegree(dir, -transform->Forward());
    pairs[CustomEnums::AxisType::MinusForward] = minusForward;
    float right = Vector3::AngleDegree(dir, transform->Right());
    pairs[CustomEnums::AxisType::Right] = right;
    float minusRight = Vector3::AngleDegree(dir, -transform->Right());
    pairs[CustomEnums::AxisType::MinusRight] = minusRight;
    float up = Vector3::AngleDegree(dir, transform->Up());
    pairs[CustomEnums::AxisType::Up] = up;
    float minusUp = Vector3::AngleDegree(dir, -transform->Up());
    pairs[CustomEnums::AxisType::MinusUp] = minusUp;

    float minAngle = std::numeric_limits<float>::max();
    CustomEnums::AxisType axisType = CustomEnums::AxisType::Forward;
    for(auto item : pairs)
    {
        if (item.second < minAngle)
        {
            minAngle = item.second;
            axisType = item.first;
        }
    }
    return axisType;
}

float Avatar::GetAnatomicAngle(CustomEnums::AvatarJointType jointType, CustomEnums::HumanAnatomicAngleType humanAnatomicAngle)
{
    skinnedModel->UpdateTransformsFromJointMapping(transforms);

    try
    {
        AvatarJoint* avatarJoint = GetAvatarJoint(jointType);

        if (avatarJoint == nullptr)
        {
            qDebug() << "Avatar: Avatar does not contain specified AvatarJoint";
            return 0.0f;
        }

        AnatomicAngleInformation* anatomicAngleInformation = nullptr;

        if (avatarJoint->twistAnatomicAngleInformation != nullptr && avatarJoint->twistAnatomicAngleInformation->ContainsType(humanAnatomicAngle))
            anatomicAngleInformation = avatarJoint->twistAnatomicAngleInformation;
        if (avatarJoint->firstDOFAnatomicAngleInformation != nullptr && avatarJoint->firstDOFAnatomicAngleInformation->ContainsType(humanAnatomicAngle))
            anatomicAngleInformation = avatarJoint->firstDOFAnatomicAngleInformation;
        if (avatarJoint->secondDOFAnatomicAngleInformation != nullptr && avatarJoint->secondDOFAnatomicAngleInformation->ContainsType(humanAnatomicAngle))
            anatomicAngleInformation = avatarJoint->secondDOFAnatomicAngleInformation;

        if (anatomicAngleInformation == nullptr)
        {
            qDebug() << "Found no combination for " << (CustomEnums::AvatarJointType)jointType << " and " << humanAnatomicAngle;
            return 0.0f;
        }

        // CenterHip is a special case because its the root of the avatar
        if (jointType == CustomEnums::AvatarJointType::CenterHip)
            return GetAnatomicAngle(CustomEnums::AvatarJointType::Spine, humanAnatomicAngle) + GetAnatomicAngle(CustomEnums::AvatarJointType::Chest, humanAnatomicAngle);
        else
        {
            bool isTwist = anatomicAngleInformation == avatarJoint->twistAnatomicAngleInformation;

            //qDebug() << "Istwist: " << isTwist;

            int nonTwistDOFs = 0;
            if (avatarJoint->firstDOFAnatomicAngleInformation != nullptr) nonTwistDOFs++;
            if (avatarJoint->secondDOFAnatomicAngleInformation != nullptr) nonTwistDOFs++;

            //qDebug() << "Non twist DOF: " << nonTwistDOFs;

            std::vector<AnatomicAngleInformation*> chosenCombination;
            std::vector<float> chosenAngles;

            // Calculation for hingejoints (1DOF)
            if (nonTwistDOFs == 1)
            {
                float angle = 0.0f;
                if (avatarJoint->firstDOFAnatomicAngleInformation != nullptr)
                {
                    angle = CalculateCurrentAngle(*avatarJoint, avatarJoint->firstDOFAnatomicAngleInformation->rotationAxis);
                    chosenCombination.push_back(avatarJoint->firstDOFAnatomicAngleInformation);
                }
                else if (avatarJoint->secondDOFAnatomicAngleInformation != nullptr)
                {
                    angle = CalculateCurrentAngle(*avatarJoint, avatarJoint->secondDOFAnatomicAngleInformation->rotationAxis);
                    chosenCombination.push_back(avatarJoint->secondDOFAnatomicAngleInformation);
                }
                else
                {
                    qDebug() << "ERROR";
                }

                chosenAngles.push_back(angle);
            }
            // Calculation for balljoints (2DOF)
            if (nonTwistDOFs == 2)
            {
                std::vector<AnatomicAngleInformation*> rotationCombinationOne;
                rotationCombinationOne.push_back(avatarJoint->firstDOFAnatomicAngleInformation);
                rotationCombinationOne.push_back(avatarJoint->secondDOFAnatomicAngleInformation);

                std::vector<AnatomicAngleInformation*> rotationCombinationTwo;
                rotationCombinationTwo.push_back(avatarJoint->secondDOFAnatomicAngleInformation);
                rotationCombinationTwo.push_back(avatarJoint->firstDOFAnatomicAngleInformation);

                std::vector<float> rotationCombinationOneAngles = CalculateAnatomicAnglesInOrder(*avatarJoint, rotationCombinationOne);
                std::vector<float> rotationCombinationTwoAngles = CalculateAnatomicAnglesInOrder(*avatarJoint, rotationCombinationTwo);

                // Sum up the anatomic angles
                float combinationOneCombinedAngle = std::abs(rotationCombinationOneAngles[0]) + std::abs(rotationCombinationOneAngles[1]);
                float combinationTwoCombinedAngle = std::abs(rotationCombinationTwoAngles[0]) + std::abs(rotationCombinationTwoAngles[1]);

                // Check which combination needs lesser movement
                if (combinationOneCombinedAngle < combinationTwoCombinedAngle)
                {
                    for (int i = 0; i < rotationCombinationOne.size(); i++)
                    {
                        chosenCombination.push_back(rotationCombinationOne[i]);
                        chosenAngles.push_back(rotationCombinationOneAngles[i]);
                    }
                }
                else
                {
                    for (int i = 0; i < rotationCombinationOne.size(); i++)
                    {
                        chosenCombination.push_back(rotationCombinationTwo[i]);
                        chosenAngles.push_back(rotationCombinationTwoAngles[i]);
                    }
                }
            }

            // NOTE: This is currently not correct for hinge joints
            // Calculation for twist
            if (isTwist)
            {
                Quaternion currentRotation = avatarJoint->transform->Rotation();

                for (int i = 0; i < chosenCombination.size(); i++)
                {
                    Vector3 rotationAxis = avatarJoint->GetAxis(chosenCombination[i]->rotationAxis, true);
                    avatarJoint->transform->Rotate(rotationAxis, -chosenAngles[i], Transform::Space::World);
                }

                Vector3 currentTwistReference = avatarJoint->GetAxis(chosenCombination[0]->rotationAxis, false);
                Vector3 startTwistReference = avatarJoint->GetAxis(chosenCombination[0]->rotationAxis, true);

                float currentTwist = Vector3::SignedAngleDegree(currentTwistReference, startTwistReference, avatarJoint->GetAxis(avatarJoint->GetForward(), true));

                avatarJoint->transform->Rotation(currentRotation);

                chosenCombination.push_back(anatomicAngleInformation);
                chosenAngles.push_back(currentTwist);
            }

            // Return the wanted value
            int index = 0;
            for (AnatomicAngleInformation* information : chosenCombination)
            {
                if (information->ContainsType(humanAnatomicAngle))
                {
                    if (information->positiveAnatomicAngleType == humanAnatomicAngle)
                    {
                        float angle = std::max(chosenAngles[index], 0.0f);
                        return angle;
                    }
                    else
                        return std::min(chosenAngles[index], 0.0f) * -1.0f;
                }

                index++;
            }
        }

        return 0.0f;
    }
    catch (const std::exception& e)
    {
        qDebug() << "Exception: " << e.what();
        return 0.0f;
    }
}

float Avatar::CalculateCurrentAngle(AvatarJoint& avatarJoint, CustomEnums::AxisType rotationAxis)
{
    Vector3 currentForward = avatarJoint.GetAxis(avatarJoint.GetForward(), false);
    Vector3 startForward = avatarJoint.GetAxis(avatarJoint.GetForward(), true);

    Vector3 startRight = avatarJoint.GetAxis(rotationAxis, true);
    Vector3 startProjection = Vector3::ProjectOnPlane(startForward, startRight);
    Vector3 currentProjection = Vector3::ProjectOnPlane(currentForward, startRight);

    float signedAngle = Vector3::SignedAngleDegree(startProjection, currentProjection, startRight);

    return signedAngle;
}

std::vector<float> Avatar::CalculateAnatomicAnglesInOrder(AvatarJoint& avatarJoint, std::vector<AnatomicAngleInformation*> ordererAnatomicAngleInformations)
{
    std::vector<float> calculatedAngles = std::vector<float>();

    Quaternion currentRotation = avatarJoint.transform->LocalRotation();

    for(AnatomicAngleInformation* anatomicAngleInformation : ordererAnatomicAngleInformations)
    {
        float angle = CalculateCurrentAngle(avatarJoint, anatomicAngleInformation->rotationAxis);
        calculatedAngles.push_back(angle);
        Vector3 rotationAxis = avatarJoint.GetAxis(anatomicAngleInformation->rotationAxis, true);
        avatarJoint.transform->Rotate(rotationAxis, -angle, Transform::Space::World);
    }

    avatarJoint.transform->LocalRotation(currentRotation);

    return calculatedAngles;

};

AvatarJoint* Avatar::GetAvatarJoint(CustomEnums::AvatarJointType humanJointType)
{
    for (int i = 0; i < avatarJoints.size(); i++)
        if (avatarJoints[i]->humanJointType == humanJointType)
            return avatarJoints[i];

    return nullptr;
}

std::vector<AvatarJoint*> Avatar::GetAllAvatarJoints()
{
    return avatarJoints;
};

void Avatar::InitAvatarJoints()
{
    for (Transform* transform : transforms)
    {
        CustomEnums::AxisType frontalAxis = GetAxis(skinnedModel->GetRoot().GlobalTrans.forward(), transform);  // Forward
        CustomEnums::AxisType longitudinalAxis = GetAxis(skinnedModel->GetRoot().GlobalTrans.up(), transform);  // Up
        CustomEnums::AxisType transverseAxis = GetAxis(skinnedModel->GetRoot().GlobalTrans.right(), transform); // Right

        #pragma region All

        #pragma region Body

        #pragma region Body : Hips

        if (transform->Name() == "Hips")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::CenterHip, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::LateralLeft, CustomEnums::HumanAnatomicAngleType::LateralRight);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma region Body : Spine

        if (transform->Name() == "Spine")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::Spine, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::LateralLeft, CustomEnums::HumanAnatomicAngleType::LateralRight);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        if (transform->Name() == "Spine1")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::Chest, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::LateralLeft, CustomEnums::HumanAnatomicAngleType::LateralRight);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "Spine2")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::UpperChest, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::LateralLeft, CustomEnums::HumanAnatomicAngleType::LateralRight);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma region Left Arm

        if (transform->Name() == "LeftShoulder")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftUpperChest, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis,CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Abduktion, CustomEnums::HumanAnatomicAngleType::Adduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "LeftArm")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftShoulder, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Adduktion, CustomEnums::HumanAnatomicAngleType::Abduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "LeftForeArm")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftElbow, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "LeftHand")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftWrist, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RadialAbduktion, CustomEnums::HumanAnatomicAngleType::UlnarAbduktion);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma region Right Arm

        if (transform->Name() == "RightShoulder")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightUpperChest, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Abduktion, CustomEnums::HumanAnatomicAngleType::Adduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "RightArm")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightShoulder, transform);

            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Abduktion, CustomEnums::HumanAnatomicAngleType::Adduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);

            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "RightForeArm")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightElbow, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "RightHand")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightWrist, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::UlnarAbduktion, CustomEnums::HumanAnatomicAngleType::RadialAbduktion);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma region Left Leg

        if (transform->Name() == "LeftUpLeg")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftHip, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Adduktion, CustomEnums::HumanAnatomicAngleType::Abduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "LeftLeg")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftKnee, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "LeftFoot")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::LeftAnkle, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationOutside, CustomEnums::HumanAnatomicAngleType::RotationInside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Adduktion, CustomEnums::HumanAnatomicAngleType::Abduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma region Right Leg

        if (transform->Name() == "RightUpLeg")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightHip, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationInside, CustomEnums::HumanAnatomicAngleType::RotationOutside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Abduktion, CustomEnums::HumanAnatomicAngleType::Adduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "RightLeg")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightKnee, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationInside, CustomEnums::HumanAnatomicAngleType::RotationOutside);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "RightFoot")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::RightAnkle, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationInside, CustomEnums::HumanAnatomicAngleType::RotationOutside);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::Abduktion, CustomEnums::HumanAnatomicAngleType::Adduktion);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Extension, CustomEnums::HumanAnatomicAngleType::Flexion);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma region Head

        if (transform->Name() == "Neck")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::Neck, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::LateralLeft, CustomEnums::HumanAnatomicAngleType::LateralRight);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        if (transform->Name() == "Head")
        {
            AvatarJoint* avatarJoint = new AvatarJoint(CustomEnums::AvatarJointType::Head, transform);
            avatarJoint->twistAnatomicAngleInformation = new AnatomicAngleInformation(longitudinalAxis, CustomEnums::HumanAnatomicAngleType::RotationLeft, CustomEnums::HumanAnatomicAngleType::RotationRight);
            avatarJoint->firstDOFAnatomicAngleInformation = new AnatomicAngleInformation(frontalAxis, CustomEnums::HumanAnatomicAngleType::LateralLeft, CustomEnums::HumanAnatomicAngleType::LateralRight);
            avatarJoint->secondDOFAnatomicAngleInformation = new AnatomicAngleInformation(transverseAxis, CustomEnums::HumanAnatomicAngleType::Flexion, CustomEnums::HumanAnatomicAngleType::Extension);
            avatarJoints.push_back(avatarJoint);
        }

        #pragma endregion

        #pragma endregion
    }
}

std::vector<CustomEnums::HumanAnatomicAngleType> Avatar::GetAnatomicAngleTypesOfJoint(CustomEnums::AvatarJointType humanJointType)
{
    AvatarJoint* avatarJoint = GetAvatarJoint(humanJointType);

    if (avatarJoint == nullptr)
    {
        //qDebug() << "Avatar does not contain joint " << CustomEnums::AvatarJointType << "\n";
        return { };
    }

    std::vector<CustomEnums::HumanAnatomicAngleType> humanAnatomicAngleTypes;

    if (avatarJoint->twistAnatomicAngleInformation != nullptr)
    {
        humanAnatomicAngleTypes.push_back(avatarJoint->twistAnatomicAngleInformation->positiveAnatomicAngleType);
        humanAnatomicAngleTypes.push_back(avatarJoint->twistAnatomicAngleInformation->negativeAnatomicAngleType);
    }

    if (avatarJoint->firstDOFAnatomicAngleInformation != nullptr)
    {
        humanAnatomicAngleTypes.push_back(avatarJoint->firstDOFAnatomicAngleInformation->positiveAnatomicAngleType);
        humanAnatomicAngleTypes.push_back(avatarJoint->firstDOFAnatomicAngleInformation->negativeAnatomicAngleType);
    }

    if (avatarJoint->secondDOFAnatomicAngleInformation != nullptr)
    {
        humanAnatomicAngleTypes.push_back(avatarJoint->secondDOFAnatomicAngleInformation->positiveAnatomicAngleType);
        humanAnatomicAngleTypes.push_back(avatarJoint->secondDOFAnatomicAngleInformation->negativeAnatomicAngleType);
    }

    return humanAnatomicAngleTypes;
}

int Avatar::GetJointCount()
{
    return avatarJoints.size();
}
