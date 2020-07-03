#pragma once

#include "MeshModel.h"
#include "Quaternion.h"
#include "Animation.h"
#include "AnimationCurve.h"
#include "Transform.h"

#define MaxBoneCount 100

class SkinnedModel : public MeshModel
{
public:
	SkinnedModel();
	SkinnedModel(const char* ModelFile, bool FitSize = false);
	virtual ~SkinnedModel();
	virtual bool load(const char* ModelFile, bool FitSize = false);
	
	virtual void draw(const BaseCamera& Cam);
	virtual void shader(BaseShader* shader, bool deleteOnDestruction = false);
	const Matrix* getBones(const int*& boneCount) const;
	std::map<std::string, JointInfo>& GetJointMapping();
	void UpdateBoneAnimation();
	std::vector<Transform*> ConvertRepresentationToTransforms();
	void SetDefaultPose();
	void SetIdentityPose();
	void SetIdentityPose(const MeshModel::Node* node, const Matrix& localToMeshTransform);
	void SetDefaultPose(const MeshModel::Node* node, const Matrix& parentTransform);

	void UpdateTransformsFromJointMapping(const std::vector<Transform*>& transforms);
	void UpdateJointMappingFromTransforms(const std::vector<Transform*>& transforms);
protected:
	GLint BonesLoc[MaxBoneCount];
	Matrix Bones[MaxBoneCount];
	int boneCount;
	
private:
	virtual void activate();
	virtual void deactivate();
	
	std::string filename;
};