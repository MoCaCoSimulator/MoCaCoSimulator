#include "AnimationCurve.h"
#include <QDebug>
#include "Customizable/JointnameParser.h"

AnimationCurve::AnimationCurve(aiNodeAnim* aiNodeAnim)
{
	name = JointnameParser::ExtractJointName(aiNodeAnim->mNodeName);

	for (int k = 0; k < aiNodeAnim->mNumPositionKeys; k++)
		positions.push_back(VectorAnimationKey(aiNodeAnim->mPositionKeys[k]));
	for (int k = 0; k < aiNodeAnim->mNumRotationKeys; k++)
		rotations.push_back(QuaternionAnimationKey(aiNodeAnim->mRotationKeys[k]));
	for (int k = 0; k < aiNodeAnim->mNumScalingKeys; k++)
		scalings.push_back(VectorAnimationKey(aiNodeAnim->mScalingKeys[k]));
}