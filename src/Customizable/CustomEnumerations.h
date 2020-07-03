#pragma once

#include <QWidget>

class CustomEnums
{
	Q_GADGET
private:
	CustomEnums() { };
public:
	enum SomeEnum { Default, Foo, Bar };
	enum IMUModel { Ideal, Orient3 };
	enum OrientationFilter { GyroIntegrator, OrientCF, YunEKF, BachmannCF };

	enum AvatarJointType
	{
		CenterHip,
		Spine,
		Chest,
		UpperChest,
		LeftShoulder,
		LeftUpperChest,
		LeftElbow,
		LeftWrist,
		RightShoulder,
		RightUpperChest,
		RightElbow,
		RightWrist,
		LeftHip,
		LeftKnee,
		LeftAnkle,
		RightHip,
		RightKnee,
		RightAnkle,
		Neck,
		Head
	};

	enum class AxisType
	{
		Forward,
		Up,
		Right,
		MinusForward,
		MinusUp,
		MinusRight
	};

	enum class HumanAnatomicAngleType
	{
		RadialAbduktion,
		UlnarAbduktion,
		Flexion,
		Extension,
		Abduktion,
		Adduktion,
		RotationInside,
		RotationOutside,
		RotationLeft,
		RotationRight,
		RotationForward,
		RotationBackward,
		Eversion,
		Inversion,
		LateralRight,
		LateralLeft,
		Protaktion,
		Retraktion,
		Elevation,
		Depression,

		All
	};

	Q_ENUM(AvatarJointType);
	Q_ENUM(SomeEnum)
	Q_ENUM(IMUModel)
	Q_ENUM(OrientationFilter)
	Q_ENUM(AxisType)
	Q_ENUM(HumanAnatomicAngleType)
};