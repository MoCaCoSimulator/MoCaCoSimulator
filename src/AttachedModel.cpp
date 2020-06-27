#include "AttachedModel.h"
#include "Paths.h"
#include <string>
#include "AttachedModelShader.h"

AttachedModel::~AttachedModel()
{

}

void AttachedModel::SumWeights(std::map<int, float>& weightMapping, const VertexBuffer::JointWeights& weights, float factor) const
{
	for (int i = 0; i < JOINTCOUNT; i++)
	{
		float weight = weights.weights[i];
		if (weight == 0.0f)
			break;

		int id = weights.ids[i];

		if (weightMapping.count(id) <= 0)
			weightMapping[id] = 0.0f;
		weightMapping[id] += weight * factor;
	}
}

std::map<int, float> AttachedModel::GenerateWeightMapping(const Vector3& position, HitInfo::TriangleInfo& triangleInfo, const std::vector<VertexBuffer::JointWeights>& joints)
{
	std::map<int, float> weightMapping;

	Vector3 pA = triangleInfo.posA - position;
	Vector3 pB = triangleInfo.posB - position;
	Vector3 pC = triangleInfo.posC - position;

	float a = (triangleInfo.posA - triangleInfo.posB).cross(triangleInfo.posA - triangleInfo.posC).length();
	float fA = pB.cross(pC).length() / a;
	float fB = pC.cross(pA).length() / a;
	float fC = pA.cross(pB).length() / a;

	SumWeights(weightMapping, joints.at(triangleInfo.indexA), fA);
	SumWeights(weightMapping, joints.at(triangleInfo.indexB), fB);
	SumWeights(weightMapping, joints.at(triangleInfo.indexC), fC);

	return weightMapping;
}

static const AttachedModel* savedAttached = NULL;
static const AttachedModel* loadedAttached = NULL;

bool AttachedModel::attachToMesh(const BaseModel* model, unsigned int meshID, const Vector3& position, HitInfo::TriangleInfo& triangleInfo, const Vector3& forward)
{
	savedAttached = this;
	const SkinnedModel* animModel = dynamic_cast<const SkinnedModel*>(model);

	//TODO implement different cases
	if (!animModel)
		return false;
	//this->parent = animModel;
	
	Vector3 u = triangleInfo.normal();
	Vector3 r = u.cross(forward);
	//backup
	if (abs(r.dot(u)) > 0.9999f)
		r = u.cross(Vector3::up);
	if (abs(r.dot(u)) > 0.9999f)
		r = u.cross(Vector3::forward);

	r.normalize();
	Vector3 f = r.cross(u);
	f.normalize();

	Matrix globalTrans = Matrix().translation(position) * Matrix().rotation(f, u, r) * getGlobalTransform().scaleMatrix();
	const Mesh& mesh = animModel->GetMesh(meshID);
	std::map<int, float> weightMapping = GenerateWeightMapping(position, triangleInfo, mesh.VB.jointWeights());

	this->attachedTo = animModel;
	this->Bones = animModel->getBones(this->boneCount);
	this->parentNode = mesh.node;
	this->transform = Matrix(this->parentNode->GlobalTrans).invert() * globalTrans;

	ApplyWeightMapping(weightMapping);

	return true;
}

bool AttachedModel::attachToMesh(const BaseModel* model, const std::string& nodeName, const std::map<int, float>& weightMapping, const Matrix& transform)
{
	loadedAttached = this;
	//AttachedModelShader* shader = new AttachedModelShader();
	//this->shader(shader, true);

	const SkinnedModel* animModel = dynamic_cast<const SkinnedModel*>(model);

	//this->parent = animModel;
	this->attachedTo = animModel;
	this->Bones = animModel->getBones(this->boneCount);
	this->parentNode = animModel->GetNode(nodeName);
	this->transform = transform;
	ApplyWeightMapping(weightMapping);

	return true;
}

std::map<int, float> AttachedModel::GetWeightMapping()
{
	std::map<int, float> sortedWeightMapping;

	for (int i = 0; i < JOINTCOUNT; i++) 
		if (attachedJointWeights[i] > 0.0f)
			sortedWeightMapping[attachedJointIndices[i]] = attachedJointWeights[i];

	return sortedWeightMapping;
}

void AttachedModel::ApplyWeightMapping(const std::map<int, float>& weightMapping)
{
	float max = std::numeric_limits<float>::max();
	for (int i = 0; i < JOINTCOUNT; i++)
	{
		float highest = 0.0f;
		int index = 0;
		//for (std::map<int, float>::iterator it = weightMapping.begin(); it != weightMapping.end(); ++it)
		for (const auto& kv : weightMapping)
		{
			const float& value = kv.second;

			if (value <= highest || value >= max)
				continue;

			highest = value;
			index = kv.first;
		}

		if (highest == 0.0f)
			break;

		max = highest;
		attachedJointWeights[i] = highest;
		attachedJointIndices[i] = index;
	}
}

Matrix AttachedModel::getAnimationTransform() const
{
	Matrix boneTransform = Matrix::zero;

	Matrix localToNode = parentNode->Trans * transform;
	Matrix nodeToLocal = Matrix(localToNode).invert();
	
	for (int i = 0; i < JOINTCOUNT; i++)
	{
		int index = attachedJointIndices[i];
		float weight = attachedJointWeights[i];

		if (weight == 0.0f)
			break;

		Matrix bone = nodeToLocal * Bones[index] * localToNode;
		boneTransform = boneTransform + bone * weight;
	}
	
	Matrix t = getGlobalTransform() * boneTransform;
	return t.lastElementDivision();
}

void AttachedModel::shader(BaseShader* shader, bool deleteOnDestruction)
{
	BaseModel::shader(shader, deleteOnDestruction);
	loadBoneArray();

	std::string name;
	for (int i = 0; i < JOINTCOUNT; i++)
	{
		std::string indexString = std::to_string(i);
		name = "AttachedJointWeights[" + indexString + "]";
		AttachedJointWeightsLoc[i] = pShader->getParameterID(name.c_str());
		name = "AttachedJointIDs[" + indexString + "]";
		AttachedJointIndicesLoc[i] = pShader->getParameterID(name.c_str());
		attachedJointWeights[i] = 0.0f;
		attachedJointIndices[i] = 0;
	}
}

void AttachedModel::draw(const BaseCamera& Cam)
{
	MeshModel::draw(Cam);
}

void AttachedModel::activate()
{
	if (!attachedTo)
		return;

	Matrix localToNode = parentNode->Trans * transform;
	Matrix nodeToLocal = Matrix(localToNode).invert();

	for (int i = 0; i < (*boneCount); ++i)
		pShader->setParameter(BonesLoc[i], nodeToLocal * Bones[i] * localToNode);

	for (int i = 0; i < JOINTCOUNT; i++)
	{
		pShader->setParameter(AttachedJointWeightsLoc[i], attachedJointWeights[i]);
		pShader->setParameter(AttachedJointIndicesLoc[i], attachedJointIndices[i]);
	}
}

const Matrix AttachedModel::localToWorld() const
{
	if (parentNode)
		return parentNode->GlobalTrans;
	return Matrix::identity;
}

void AttachedModel::loadBoneArray()
{
	for (int i = 0; i < MaxBoneCount; i++)
	{
		std::string name = "Bones[" + std::to_string(i) + "]";
		BonesLoc[i] = pShader->getParameterID(name.c_str());
	}
}