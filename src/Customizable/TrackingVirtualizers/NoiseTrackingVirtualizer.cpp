#include "NoiseTrackingVirtualizer.h"
#define _USE_MATH_DEFINES
#include <math.h>

RegisterVirtualizer<NoiseTrackingVirtualizer> NoiseTrackingVirtualizer::Register;

bool NoiseTrackingVirtualizer::randInitialized = false;
int NoiseTrackingVirtualizer::randSeed = 0;

NoiseTrackingVirtualizer::NoiseTrackingVirtualizer() : BaseTrackingVirtualizer("NoiseTrackingVirtualizer")
{
	AddParameter(new Parameter<float>("NoiseStrength", 0.0f));
	AddParameter(new Parameter<int>("RandomSeed", 0));
	AddParameter(new Parameter<int>("SampleRate", 30));
}

bool NoiseTrackingVirtualizer::CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output)
{
	Parameter<float>* noiseStrength = dynamic_cast<Parameter<float>*>(parameters["NoiseStrength"]);
	Parameter<int>* seed = dynamic_cast<Parameter<int>*>(parameters["RandomSeed"]);
	Parameter<int>* sampleRate = dynamic_cast<Parameter<int>*>(parameters["SampleRate"]);
	InitializeRand(seed->GetValue());

	float sampleCount = trackerHandle.GetAnimationLength() * sampleRate->GetValue();

	for (int sample = 0; sample <= sampleCount; ++sample)
	{
		float random = std::rand();
		Vector3 posOffset = Vector3(std::rand(), std::rand(), std::rand());
		posOffset.normalize();
		posOffset = posOffset * noiseStrength->GetValue();
		Vector3 rotOffset = Vector3(std::rand(), std::rand(), std::rand());
		rotOffset = rotOffset * noiseStrength->GetValue() * M_PI;
		Quaternion quatOffset = Quaternion(rotOffset);

		float timeNormalized = std::min(sample / sampleCount, 1.0f);
		Matrix transform = trackerHandle.GetTransform(timeNormalized);

		float time = timeNormalized * trackerHandle.GetAnimationLength();
		qDebug() << "save" << "pos" << transform.translation().toString().c_str() << "rot" << (transform.rotation() * quatOffset).eulerAngles().toString().c_str();
		output.positions.push_back(AnimationCurve::VectorAnimationKey(time, transform.translation() + posOffset));
		output.rotations.push_back(AnimationCurve::QuaternionAnimationKey(time, transform.rotation() * quatOffset));
		output.scalings.push_back(AnimationCurve::VectorAnimationKey(time, transform.scale()));
	}

	return true;
}

void NoiseTrackingVirtualizer::InitializeRand(int seed)
{
	if (randInitialized && seed == randSeed)
		return;
	
	std::srand(seed);
	randSeed = seed;
	randInitialized = true;
}

BaseTrackingVirtualizer* NoiseTrackingVirtualizer::Clone() const
{
	return new NoiseTrackingVirtualizer();
}
