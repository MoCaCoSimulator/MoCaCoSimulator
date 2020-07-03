
#pragma once
#ifndef AttachedModel_h
#define AttachedModel_h

#include "SkinnedModel.h"

class AttachedModel : public MeshModel
{
public:
	AttachedModel() : MeshModel(), center(Vector3::zero), parentNode(NULL), attachedTo(NULL), Bones(NULL), boneCount(NULL) {};
	AttachedModel(const char* ModelFile, bool FitSize = false) : 
		MeshModel(ModelFile, FitSize), 
		center(Vector3::zero), 
		parentNode(NULL), 
		attachedTo(NULL), 
		Bones(NULL), 
		boneCount(NULL)
	{};
	virtual ~AttachedModel();

	bool attachToMesh(const BaseModel* model, unsigned int meshID, const Vector3& position, HitInfo::TriangleInfo& triangleInfo, const Vector3& forward = Vector3::forward);
	bool attachToMesh(const BaseModel* model, const std::string& nodeName, const std::map<int, float>& weightMapping, const Matrix& transform);
	std::map<int, float> GetWeightMapping();
	std::map<int, float> GenerateWeightMapping(const Vector3& position, HitInfo::TriangleInfo& triangleInfo, const std::vector<VertexBuffer::JointWeights>& joints);
	Matrix getAnimationTransform() const;
	virtual void draw(const BaseCamera& Cam);
	virtual void activate();
	virtual const Matrix localToWorld() const;
	virtual void shader(BaseShader* shader, bool deleteOnDestruction);
	virtual BaseShader* shader() { return BaseModel::shader(); }
	const BaseModel* getAttachedTo() const { return attachedTo; }
	const Node* getParentNode() const { return parentNode; }
	//void deactivate();
protected:
	void loadBoneArray();
protected:
	GLint BonesLoc[MaxBoneCount];
	GLuint AttachedJointWeightsLoc[JOINTCOUNT];
	GLuint AttachedJointIndicesLoc[JOINTCOUNT];

	float attachedJointWeights[JOINTCOUNT];
	int attachedJointIndices[JOINTCOUNT];

	const Matrix* Bones;
	const int* boneCount;
	Vector3 center;
	const Node* parentNode;
	const BaseModel* attachedTo;
private:
	void SumWeights(std::map<int, float>& weightMapping, const VertexBuffer::JointWeights& weights, float factor) const;
	void ApplyWeightMapping(const std::map<int, float>& weightMapping);
};

#endif /* AttachedModel_h */
