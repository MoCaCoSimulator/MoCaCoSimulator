#pragma once

#include <QWidget>

// This reaonably better version is not supported by Qt <5.8
//namespace Enumerations
//{
//	Q_NAMESPACE
//	enum class TestEnum { VL, CA, TE };
//	Q_ENUM_NS(TestEnum)
//};

// Proxy class to make enums showable in the UI
class Enums
{
	Q_GADGET
private:
	Enums() { };
public:
	enum class HumanJointType
	{
		// Spine & Head
		Hips,
		Spine,
		Spine1,
		Spine2,
		Neck,
		Head,

		// Left Leg
		LeftUpLeg,
		LeftLeg,
		LeftFoot,
		LeftToeBase,
		//LeftToe_End,

		// Right Leg
		RightUpLeg,
		RightLeg,
		RightFoot,
		RightToeBase,
		//RightToe_End,

		// Left Arm
		LeftShoulder,
		LeftArm,
		LeftForeArm,
		LeftHand,

		// Left Fingers
		/*LeftHandThumb1,
		LeftHandThumb2,
		LeftHandThumb3,
		LeftHandThumb4,
		LeftHandIndex1,
		LeftHandIndex2,
		LeftHandIndex3,
		LeftHandIndex4,
		LeftHandMiddle1,
		LeftHandMiddle2,
		LeftHandMiddle3,
		LeftHandMiddle4,
		LeftHandRing1,
		LeftHandRing2,
		LeftHandRing3,
		LeftHandRing4,
		LeftHandPinky1,
		LeftHandPinky2,
		LeftHandPinky3,
		LeftHandPinky4,*/

		// Right Arm
		RightShoulder,
		RightArm,
		RightForeArm,
		RightHand,

		// Right Fingers
		/*RightHandThumb1,
		RightHandThumb2,
		RightHandThumb3,
		RightHandThumb4,
		RightHandIndex1,
		RightHandIndex2,
		RightHandIndex3,
		RightHandIndex4,
		RightHandMiddle1,
		RightHandMiddle2,
		RightHandMiddle3,
		RightHandMiddle4,
		RightHandRing1,
		RightHandRing2,
		RightHandRing3,
		RightHandRing4,
		RightHandPinky1,
		RightHandPinky2,
		RightHandPinky3,
		RightHandPinky4,*/

		All
	};
	
	
	Q_ENUM(HumanJointType)
};