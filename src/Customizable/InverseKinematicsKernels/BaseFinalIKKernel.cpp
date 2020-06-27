#include "BaseFinalIKKernel.h"

BaseFinalIKKernel::BaseFinalIKKernel(std::string name) : BaseIKKernel(name), ikSolver(NULL)
{
	AddParameter(new Parameter<int>("SampleRate", 60));
}

Animation* BaseFinalIKKernel::GenerateSolvedAnimation(float duration)
{
	Animation* solvedAnimation = new Animation();
	solvedAnimation->duration = duration;

	for (Transform*& transform : references.GetTransforms())
	{
		if (transform->Name() == "RootNode")
			continue;
		AnimationCurve curve = AnimationCurve();
		curve.name = transform->Name();
		solvedAnimation->animNodeMapping.insert(std::pair<std::string, AnimationCurve>(curve.name, curve));
	}
	return solvedAnimation;
}

void BaseFinalIKKernel::ApplyCurveDataToTarget(Transform* target, const AnimationCurve& curve, const float& time)
{
	Vector3 pos = curve.GetPosition(time);
	Quaternion rot = curve.GetRotation(time);
	Matrix globalAnimatedTransform = Matrix().translation(pos) * rot.toRotationMatrix();

	Matrix t = globalAnimatedTransform * slotOffsetMap[curve.name];

	target->Position(t.translation().ReverseX());
	target->Rotation(t.rotation().ReverseXW());
}

void BaseFinalIKKernel::ApplyCurveDataToTarget(Transform* target, const AnimationCurve& curve, const int& index)
{
	target->Position(curve.positions[index].value.ReverseX());
	target->Rotation(curve.rotations[index].value.ReverseXW());
}

void BaseFinalIKKernel::RunSolver(float deltaTime)
{
	ikSolver->FixTransforms();
	ikSolver->Update(deltaTime);
}

void BaseFinalIKKernel::IterateData(const Animation& endEffectorsAnimation, Animation*& solvedAnimation)
{
	int sampleRate = dynamic_cast<Parameter<int>*>(parameters["SampleRate"])->GetValue();
	float animDuration = endEffectorsAnimation.duration;
	float sampleCount = sampleRate * animDuration;
	float deltaTime = 1.0f / sampleRate;

	//optional compute additional last frame, would not be within sample rate range
	for (int sample = 0; sample < sampleCount/* + 1*/; ++sample)
	{
		float time = sample * animDuration / sampleCount;
		//time = std::fmin(time, animDuration);

		ApplyCurveDataToTargets(time);
		RunSolver(deltaTime);
		SaveSolvedData(solvedAnimation, time);
	}
}

void BaseFinalIKKernel::AddTargetCurve(Transform*& target, const AnimationCurve& source)
{
	target = new Transform();
	target->Name(source.name);
	targetCurveMap[target] = source;
}

void BaseFinalIKKernel::ApplyCurveDataToTargets(const float& time)
{
	for (const auto& kv : targetCurveMap)
		ApplyCurveDataToTarget(kv.first, kv.second, time);
}

void BaseFinalIKKernel::ApplyCurveDataToTargets(const int& index)
{
	for (const auto& kv : targetCurveMap)
		ApplyCurveDataToTarget(kv.first, kv.second, index);
}

void BaseFinalIKKernel::SaveSolvedData(Animation*& solvedAnimation, const float& time)
{
	// Push the solved positions and rotations into the curresponding curves
		//for(auto pair : groundTruthAnimation.animNodeMapping)
	for (Transform*& transform : references.GetTransforms())
	{
		if (transform->Name() == "RootNode")
			continue;

		Vector3 localPosition;
		Quaternion localRotation;

		// Add root animation to hips, as root gets not animated in our system
		if (transform->Name() == "Hips")
		{
			Matrix root = references.root->LocalMatrix();
			//remove scale from root matrix
			root = root.translationMatrix() * root.rotationMatrix();
			Matrix mat = root * transform->LocalMatrix();
			localPosition = mat.translation();
			localRotation = mat.rotation();
		}
		else
		{
			localPosition = transform->LocalPosition();
			localRotation = transform->LocalRotation();
		}
		
		AnimationCurve& animCurve = solvedAnimation->animNodeMapping[transform->Name()];
		animCurve.positions.push_back(AnimationCurve::VectorAnimationKey(time, localPosition.ReverseX()));
		animCurve.rotations.push_back(AnimationCurve::QuaternionAnimationKey(time, localRotation.ReverseXW()));
		animCurve.scalings.push_back(AnimationCurve::VectorAnimationKey(time, transform->LocalScale()));
	}
}

void BaseFinalIKKernel::GenerateOffsets(std::map<std::string, Tracker*> trackers)
{
	for (const auto& kv : trackers)
	{
		Tracker* tracker = kv.second;
		slotOffsetMap[kv.first] = tracker->GetOffsetMatrix();
	}
}

void BaseFinalIKKernel::InitSolver(SkinnedModel& model)
{
	//set modelpose to default pose (probably t-pose)
	Animator tempAnimator = Animator(model);
	tempAnimator.RemoveAnimation();

	transforms = model.ConvertRepresentationToTransforms();

	Transform* root = nullptr;
	for (Transform* transform : transforms)
		if (transform->Name() == "RootNode")
			root = transform;

	if (root == nullptr)
		qDebug() << "ERROR";

	ikSolver = new RootMotion::IKSolverVR();

	references = RootMotion::VRIK::References();

	for (Transform* transform : transforms)
	{
		std::string name = transform->Name();
		if (name == "RootNode")
			references.root = transform; // 0
		else if (name == "Hips")
			references.pelvis = transform; // 1
		else if (name == "Spine")
			references.spine = transform; // 24
		else if (name == "Spine1")
			references.chest = transform; // 3 Optional
		else if (name == "Neck")
			references.neck = transform; // 4 Optional
		else if (name == "Head")
			references.head = transform; // 5
		else if (name == "LeftShoulder")
			references.leftShoulder = transform; // 6 Optional
		else if (name == "LeftArm")
			references.leftUpperArm = transform; // 7
		else if (name == "LeftForeArm")
			references.leftForearm = transform; // 8
		else if (name == "LeftHand")
			references.leftHand = transform; // 9
		else if (name == "RightShoulder")
			references.rightShoulder = transform; // 10 Optional
		else if (name == "RightArm")
			references.rightUpperArm = transform; // 11
		else if (name == "RightForeArm")
			references.rightForearm = transform; // 12
		else if (name == "RightHand")
			references.rightHand = transform; // 13
		else if (name == "LeftUpLeg")
			references.leftThigh = transform; // 14 Optional
		else if (name == "LeftLeg")
		{
			transform->Rotate(Vector3::right, 5.0f, Transform::Space::World);
			references.leftCalf = transform; // 15 Optional
		}
		else if (name == "LeftFoot")
			references.leftFoot = transform; // 16 Optional
		else if (name == "LeftToeBase")
			references.leftToes = transform; // 17 Optional
		else if (name == "RightUpLeg")
			references.rightThigh = transform; // 18 Optional
		else if (name == "RightLeg")
		{
			transform->Rotate(Vector3::right, 5.0f, Transform::Space::World);
			references.rightCalf = transform; // 19 Optional
		}
		else if (name == "RightFoot")
			references.rightFoot = transform; // 20 Optional
		else if (name == "RightToeBase")
			references.rightToes = transform; // 21 Optional
	}

	// Set the references to the solver
	ikSolver->SetToReferences(references);
	ikSolver->Initiate(root);

	ikSolver->plantFeet = false;
	ikSolver->locomotion->weight = 0.0f;
	ikSolver->spine->minHeadHeight = 0.0f;
}

void BaseFinalIKKernel::ReleaseTransforms()
{
	for (Transform* transform : transforms)
		delete transform;
	transforms.clear();
}

void BaseFinalIKKernel::ReleaseTargets()
{
	for (const auto& kv : targetCurveMap)
		delete kv.first;
	targetCurveMap.clear();
}