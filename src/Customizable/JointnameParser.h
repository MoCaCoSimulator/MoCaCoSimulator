#pragma once

#include <map>
#include <string>
#include <assimp\types.h>
#include <vector>

class JointnameParser
{
public:
	enum Joints
	{
		Chin,
		Head,
		Hips,
		Jaw,
		L_CheekFold,
		L_Ear,
		L_Eye,
		L_EyelidLower,
		L_EyelidUpper,
		L_IOuterBrow,
		L_InnerBrow,
		L_InnerCheek,
		L_LipCorner,
		L_LipCornerLowTweak,
		L_LipCornerUpTweak,
		L_LipLower,
		L_LipUpper,
		L_LowerCheek,
		L_MidBrow,
		L_Nostril,
		L_OuterCheek,
		L_Temple,
		LeftArm,
		LeftFoot,
		LeftForeArm,
		LeftHand,
		LeftHandIndex1,
		LeftHandIndex2,
		LeftHandIndex3,
		LeftHandMiddle1,
		LeftHandMiddle2,
		LeftHandMiddle3,
		LeftHandPinky1,
		LeftHandPinky2,
		LeftHandPinky3,
		LeftHandRing1,
		LeftHandRing2,
		LeftHandRing3,
		LeftHandThumb1,
		LeftHandThumb2,
		LeftHandThumb3,
		LeftLeg,
		LeftShoulder,
		LeftToeBase,
		LeftUpLeg,
		LipMidLower,
		LipMidUpper,
		LowerChin,
		MidBrows,
		Neck,
		Neck1,
		R_CheekFold,
		R_Ear,
		R_Eye,
		R_EyelidLower,
		R_EyelidUpper,
		R_IOuterBrow,
		R_InnerBrow,
		R_InnerCheek,
		R_LipLower,
		R_LipUpper,
		R_LowerCheek,
		R_MidBrow,
		R_OuterCheek,
		R_Temple,
		R_gLipCorner,
		R_gLipCornerLowTweak,
		R_gLipCornerUpTweak,
		RightArm,
		RightFoot,
		RightForeArm,
		RightHand,
		RightHandIndex1,
		RightHandIndex2,
		RightHandIndex3,
		RightHandMiddle1,
		RightHandMiddle2,
		RightHandMiddle3,
		RightHandPinky1,
		RightHandPinky2,
		RightHandPinky3,
		RightHandRing1,
		RightHandRing2,
		RightHandRing3,
		RightHandThumb1,
		RightHandThumb2,
		RightHandThumb3,
		RightLeg,
		RightNostril,
		RightShoulder,
		RightToeBase,
		RightUpLeg,
		Scalp,
		Spine,
		Spine1,
		Spine2,
		Throat,
		TongueBack,
		TongueMid,
		TongueTip,
	};

	static std::vector<std::string> expectedNames;
	static std::map<std::string, std::string> jointnameMap;
	static std::map<Joints, std::string> jointNames;
	static std::string delimiter;

	static void GetJoints();
	static void UseMapping(std::map<std::string, std::string> map);

	static std::string ExtractJointName(const aiString& aiName);	
};