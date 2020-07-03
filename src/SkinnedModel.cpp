#include "SkinnedModel.h"
#include "paths.h"
#include <QDebug>

SkinnedModel::SkinnedModel() : boneCount(0)
{
}

SkinnedModel::SkinnedModel(const char* ModelFile, bool FitSize) : MeshModel()
{
	bool ret = load(ModelFile, FitSize);
	if (!ret)
		throw std::exception();
}

SkinnedModel::~SkinnedModel()
{
}

bool SkinnedModel::load(const char* ModelFile, bool FitSize)
{
	bool ret = MeshModel::load(ModelFile, FitSize);
	if (!ret)
		return false;

	boneCount = (int)jointMapping.size();
	for (int i = 0; i < MaxBoneCount; i++)
		Bones[i].setIdentity();

	SetDefaultPose();
	return true;
}

void SkinnedModel::shader(BaseShader* shader, bool deleteOnDestruction)
{
	BaseModel::shader(shader, deleteOnDestruction);

	for (int i = 0; i < MaxBoneCount; i++)
	{
		std::string name = "Bones[" + std::to_string(i) + "]";
		BonesLoc[i] = pShader->getParameterID(name.c_str());
	}
}

const Matrix* SkinnedModel::getBones(const int*& boneCount) const
{
	boneCount = &this->boneCount;
	return Bones;
}

std::map<std::string, SkinnedModel::JointInfo>& SkinnedModel::GetJointMapping()
{
	return jointMapping;
}

void SkinnedModel::activate()
{
	for (int i = 0; i < boneCount; ++i)
		pShader->setParameter(BonesLoc[i], Bones[i]);
}

void SkinnedModel::deactivate()
{

}

void SkinnedModel::UpdateBoneAnimation()
{
	for (std::map<std::string, SkinnedModel::JointInfo>::iterator it = jointMapping.begin(); it != jointMapping.end(); ++it)
	{
		JointInfo& jointInfo = it->second;
		Bones[jointInfo.jointID] = jointInfo.transform;
	}
}

void SkinnedModel::UpdateTransformsFromJointMapping(const std::vector<Transform*>& transforms)
{
	for (Transform* transform : transforms)
	{
		if (transform->Name() == "RootNode")
			continue;

		JointInfo& info = jointMapping[transform->Name()];
		transform->SetLocalMatrix(info.localTransform);
		transform->Reverse();
	}
}

void SkinnedModel::UpdateJointMappingFromTransforms(const std::vector<Transform*>& transforms)
{
	for (Transform* transform : transforms)
	{
		if (transform->Name() == "RootNode")
			continue;

		JointInfo& info = jointMapping[transform->Name()];
		info.transform = inverseMeshTransform * transform->GlobalMatrixReversed() * info.offset;
	}
}

std::vector<Transform*> SkinnedModel::ConvertRepresentationToTransforms()
{
	std::vector<Transform*> transforms = std::vector<Transform*>();

	// Create all the joints
	for (std::pair<const std::string, MeshModel::JointInfo>& kv : jointMapping)
	{
		//qDebug() << kv.first.c_str() << kv.second.node;
		std::string name = kv.second.node->Name;
		Transform* transform = new Transform(kv.second.localTransform);
		transform->Reverse();
		transform->Name(name);
		transforms.push_back(transform);
	}

	// Create the root
	Transform* rootTransform = new Transform(RootNode.Trans);
	rootTransform->Reverse();
	rootTransform->Name("RootNode");
	transforms.push_back(rootTransform);

	// Setup the hierarchy
	for (Transform* transform : transforms)
	{
		if (transform == rootTransform)
			continue;

		const MeshModel::Node* node = jointMapping.at(transform->Name()).node;

		if (!node->Parent)
			continue;

		for (Transform* t : transforms)
		{
			if (t->Name() == node->Parent->Name)
			{
				transform->SetParent(t);
				break;
			}
		}
	}

	return transforms;
}

void SkinnedModel::SetIdentityPose()
{
	SetIdentityPose(&RootNode, Matrix::identity);
	UpdateBoneAnimation();
}

void SkinnedModel::SetIdentityPose(const MeshModel::Node* node, const Matrix& localToMeshTransform)
{
	std::string nodeName = node->Name;
	Matrix meshTransform = localToMeshTransform * node->Trans;

	auto& jointMapping = GetJointMapping();
	if (jointMapping.find(nodeName) != jointMapping.end())
	{
		SkinnedModel::JointInfo& info = jointMapping.at(nodeName);

		//override meshtransform for identity matrix insertion
		meshTransform = RootNode.Trans * Matrix(info.offset).invert();

		info.transform = Matrix::identity;
		info.localTransform = Matrix(localToMeshTransform).invert() * meshTransform;
		info.globalTransform = getGlobalTransform() * meshTransform;
	}

	for (int i = 0; i < node->ChildCount; i++)
		SetIdentityPose(&node->Children[i], meshTransform);
}

void SkinnedModel::SetDefaultPose()
{
	SetDefaultPose(&RootNode, Matrix::identity);
	UpdateBoneAnimation();
}

void SkinnedModel::SetDefaultPose(const MeshModel::Node* node, const Matrix& localToMeshTransform)
{
	std::string nodeName = node->Name;
	Matrix meshTransform = localToMeshTransform * node->Trans;

	auto& jointMapping = GetJointMapping();
	if (jointMapping.find(nodeName) != jointMapping.end())
	{
		SkinnedModel::JointInfo& info = jointMapping.at(nodeName);

		info.transform = inverseMeshTransform * meshTransform * info.offset;
		info.localTransform = node->Trans;
		info.globalTransform = getGlobalTransform() * meshTransform;
	}

	for (int i = 0; i < node->ChildCount; i++)
		SetDefaultPose(&node->Children[i], meshTransform);
}

void SkinnedModel::draw(const BaseCamera& Cam)
{
	MeshModel::draw(Cam);
}