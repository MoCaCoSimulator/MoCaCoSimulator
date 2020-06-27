#include "Animator.h"
#include <algorithm>
/*
Animator::Animator(const MeshModel::Node& rootNode, std::map<std::string, SkinnedModel::JointInfo>& jointMapping, Animation* animation) :
RootNode(&rootNode),
globalInverseTransform(RootNode->Trans),
jointMapping(&jointMapping),
animation(animation),
animationTime(0.0f),
isPlaying(false), speed(1.0f),
loopAnimation(false)
{
	globalInverseTransform.invert();
}
*/
Animator::Animator(SkinnedModel& model, const Animation* animation) :
	model(&model),
	animation(animation),
	animationTime(0.0f),
	isPlaying(false), speed(1.0f),
	loopAnimation(false)
{
	//RootNode = &model.GetRoot();
	//globalInverseTransform = RootNode->Trans;
	//globalInverseTransform.invert();
}

Animator::~Animator()
{
	if (animation)
		delete animation;
}

void Animator::Update(float deltaTime)
{
	if (!animation)
		return;
	
	if (isPlaying)
	{
		float time = animationTime + deltaTime * speed;

		// Check if the animation is finished
		if (time > GetAnimationLength())
		{
			if (loopAnimation)
				time -= GetAnimationLength();
			else
			{
				isPlaying = false;
				time = GetAnimationLength();
			}
		}

		SetAnimationTime(time);
	}
}

const Animation* Animator::GetAnimation() const
{
	return animation;
}

void Animator::Reset()
{
	this->isPlaying = false;
	this->SetAnimationTime(0.0f, true);
}

void Animator::SetAnimation(Animation* newAnimation)
{
	if (newAnimation != animation)
		this->animation = newAnimation;
	Reset();
}

void Animator::RemoveAnimation(bool destroy)
{
	if (!animation)
		return;

	if (destroy)
		delete animation;

	animation = NULL;
	this->DefaultPose();
	Reset();
}

void Animator::SetAnimationTime(float time, bool force)
{
	if (!animation) {
		animationTime = time;
		model->UpdateBoneAnimation();
		return;
	}
	
	if (!force && animationTime == time)
		return;

	animationTime = time;
	UpdateAnimation(animationTime/* * animation->ticksPerSecond*/, &model->GetRoot(), Matrix::identity);
	model->UpdateBoneAnimation();
}

void Animator::SetNormalizedAnimationTime(float normalizedTime)
{
	if (!animation)
		return;

	normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);
	SetAnimationTime(normalizedTime * animation->duration);
}

void Animator::Play(const bool loop)
{
	if (!animation)
		return;

	isPlaying = true;
}

void Animator::Pause()
{
	if (!animation)
		return;

	isPlaying = false;
}

void Animator::Stop()
{
	if (!animation)
		return;

	isPlaying = false;
	SetNormalizedAnimationTime(0.0f);
}

const MeshModel::Node* Animator::GetRootNode()
{
	return &model->GetRoot();
}

float Animator::GetAnimationLength()
{
	return animation->duration;
}

bool Animator::IsPlaying()
{
	return isPlaying;
}

float Animator::NormalizedTime()
{
	return animationTime / GetAnimationLength();
}

Quaternion Animator::InterpolateQuaternion(float time, const std::vector<AnimationCurve::QuaternionAnimationKey>& quatKey) const
{
	return AnimationCurve::QuaternionAnimationKey::Interpolate(time, quatKey);
}

Vector3 Animator::InterpolateVector(float time, const std::vector<AnimationCurve::VectorAnimationKey>& vectorKey) const
{
	return AnimationCurve::VectorAnimationKey::Interpolate(time, vectorKey);
}

float Animator::GetInterpolationTime(const AnimationCurve::AnimationKey& from, const AnimationCurve::AnimationKey& to, const float time)
{
	return (time - from.time) / (to.time - from.time);
}

Matrix Animator::GetNodeTransform(float time, std::string nodeName)
{
	SetAnimationTime(time);
	return model->GetJointMapping().at(nodeName).transform;
}

void Animator::UpdateAnimation(float time, const MeshModel::Node* node, const Matrix& parentTransform)
{
	std::string nodeName = node->Name;
	Matrix nodeTransform = node->Trans;

	if (animation->animNodeMapping.find(nodeName) != animation->animNodeMapping.end())
	{
		const AnimationCurve& animNode = animation->animNodeMapping.at(nodeName);
		Matrix scalingMatrix = Matrix().scale(InterpolateVector(time, animNode.scalings));
		Matrix rotationMatrix = InterpolateQuaternion(time, animNode.rotations).toRotationMatrix();
		Matrix translationMatrix = Matrix().translation(InterpolateVector(time, animNode.positions));
		nodeTransform = translationMatrix * rotationMatrix * scalingMatrix;
	}

	Matrix meshTransform = parentTransform * nodeTransform;

	auto& jointMapping = model->GetJointMapping();
	if (jointMapping.find(nodeName) != jointMapping.end())
	{
		SkinnedModel::JointInfo& info = jointMapping.at(nodeName);
		info.transform = model->GetInverseMeshTransform() * meshTransform * info.offset;
		info.localTransform = nodeTransform;
		info.globalTransform = model->getGlobalTransform() * meshTransform;
	}

	for (int i = 0; i < node->ChildCount; i++)
		UpdateAnimation(time, &node->Children[i], meshTransform);
}

void Animator::DefaultPose()
{
	model->SetDefaultPose();
}

int Animator::FindActiveQuaternionKeyIndex(float time, const std::vector<AnimationCurve::QuaternionAnimationKey>& keys) const
{
	if (keys.size() == 1)
		return 0;

	for (int i = keys.size() - 2; i >= 0; i--)
	{
		if (time >= keys[i].time)
			return i;
	}

	assert(0);
	return -1;
}

int Animator::FindActiveVectorKeyIndex(float time, const std::vector<AnimationCurve::VectorAnimationKey>& keys) const
{
	if (keys.size() == 1)
		return 0;

	for (int i = keys.size() - 2; i >= 0; i--)
	{
		if (time >= keys[i].time)
			return i;
	}

	assert(0);
	return -1;
}
