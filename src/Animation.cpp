#include "Animation.h"
#include <QDebug>
#include "Utils.h"
#include "assimp/Exporter.hpp"
#include "Customizable/JointnameParser.h"

Animation::Animation(aiAnimation* aiAnim)
	:
	ticksPerSecond(aiAnim->mTicksPerSecond)
{
	duration = aiAnim->mDuration / aiAnim->mTicksPerSecond;
	for (int j = 0; j < aiAnim->mNumChannels; j++)
	{
		AnimationCurve animNode = AnimationCurve(aiAnim->mChannels[j]);
		animNodeMapping[animNode.name] = animNode;
	}
}

std::string Animation::ToString()
{
	std::string output = "";
	for (std::pair<std::string, AnimationCurve> AnimationCurve : animNodeMapping)
		output += "Nodename: " + AnimationCurve.first + "\n";
	return output;
}

Animation* Animation::LoadFromPath(const std::string& path)
{
	// Return null if file does not exist
	if (!Utils::FileExists(path))
	{
		qDebug() << "Could not load animation: File does not exist";
		return nullptr;
	}

	std::string fileName = Utils::FilenameFromPath(path, false, "/\\");
	
	const aiScene* pScene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords);

	if (!pScene)
	{
		qDebug() << "Invalid files";
		return nullptr;
	}
 
	if (pScene->mNumAnimations == 0)
	{
		qDebug() << "Could not load animation: File contains no animations";
		return nullptr;
	}

	Animation* animation = new Animation(pScene->mAnimations[0]);

	if (pScene->mNumAnimations == 1)
		animation->name = fileName;
	else
		animation->name = fileName + std::to_string(pScene->mNumAnimations);

	animation->filename = Utils::FilenameFromPath(path, true, "/\\");
	animation->path = path.substr(0, path.find(animation->filename, 0) - 1) + "/";

	return animation;
}

void RenameNodesToGeneric(aiNode* pAiNode)
{
	pAiNode->mName = JointnameParser::ExtractJointName(pAiNode->mName);
	for(unsigned int i = 0; i < pAiNode->mNumChildren; ++i)
		RenameNodesToGeneric(pAiNode->mChildren[i]);
}

void Animation::SaveToPath(const std::string& sourcePath, const Animation& animation, const std::string& path)
{
	aiScene* pAiScene = const_cast<aiScene*>(aiImportFile(sourcePath.c_str(), aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords));
	RenameNodesToGeneric(pAiScene->mRootNode);

	const aiAnimation* pSourceAiAnimation = pAiScene->mAnimations[0];
	aiAnimation* pAiAnim = new aiAnimation(*pSourceAiAnimation);

	std::map<std::string, AnimationCurve> animNodeMapping = animation.animNodeMapping;
	pAiAnim->mDuration = animation.duration;
	pAiAnim->mTicksPerSecond = animation.ticksPerSecond;
	pAiAnim->mNumChannels = animNodeMapping.size();
	pAiAnim->mChannels = new aiNodeAnim*[pAiAnim->mNumChannels]; // NOTE: Does this not create new entities?
	
	unsigned int i = 0;
	for(const auto& kv : animNodeMapping)
	{
		const AnimationCurve& nodeAnim = kv.second;
		aiNodeAnim* pAiNodeAnim = new aiNodeAnim();

		pAiNodeAnim->mNodeName = nodeAnim.name;
		pAiNodeAnim->mNumPositionKeys = nodeAnim.positions.size();
		pAiNodeAnim->mNumRotationKeys = nodeAnim.rotations.size();
		pAiNodeAnim->mNumScalingKeys = nodeAnim.scalings.size();

		pAiNodeAnim->mPositionKeys = new aiVectorKey[pAiNodeAnim->mNumPositionKeys];
		for (int k = 0; k < pAiNodeAnim->mNumPositionKeys; k++)
		{
			const AnimationCurve::VectorAnimationKey& animKey = nodeAnim.positions[k];
			aiVectorKey aiAnimationKey;

			aiAnimationKey.mTime = animKey.time;
			aiAnimationKey.mValue = aiVector3D(animKey.value.x, animKey.value.y, animKey.value.z);

			pAiNodeAnim->mPositionKeys[k] = aiAnimationKey;
		}

		pAiNodeAnim->mRotationKeys = new aiQuatKey[pAiNodeAnim->mNumRotationKeys];
		for (int k = 0; k < pAiNodeAnim->mNumRotationKeys; k++)
		{
			const AnimationCurve::QuaternionAnimationKey& animKey = nodeAnim.rotations[k];
			aiQuatKey aiAnimationKey;

			aiAnimationKey.mTime = animKey.time;
			aiAnimationKey.mValue = aiQuaternion(animKey.value.w, animKey.value.x, animKey.value.y, animKey.value.z);

			pAiNodeAnim->mRotationKeys[k] = aiAnimationKey;
		}

		pAiNodeAnim->mScalingKeys = new aiVectorKey[pAiNodeAnim->mNumScalingKeys];
		for (int k = 0; k < pAiNodeAnim->mNumScalingKeys; k++) {

			const AnimationCurve::VectorAnimationKey& animKey = nodeAnim.scalings[k];
			aiVectorKey aiAnimationKey;

			aiAnimationKey.mTime = animKey.time;
			aiAnimationKey.mValue = aiVector3D(animKey.value.x, animKey.value.y, animKey.value.z);

			pAiNodeAnim->mScalingKeys[k] = aiAnimationKey;
		}

		pAiAnim->mChannels[i++] = pAiNodeAnim;
	}

	pAiScene->mNumAnimations = 1;
	aiAnimation* aiAnimArray[1] = { pAiAnim };
	pAiScene->mAnimations = aiAnimArray;
	
	aiExportScene(pAiScene, "collada", path.c_str(), 0);
}
