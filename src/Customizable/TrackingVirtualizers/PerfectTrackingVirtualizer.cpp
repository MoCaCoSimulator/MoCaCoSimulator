#include "PerfectTrackingVirtualizer.h"

RegisterVirtualizer<PerfectTrackingVirtualizer> PerfectTrackingVirtualizer::Register;

PerfectTrackingVirtualizer::PerfectTrackingVirtualizer() : BaseTrackingVirtualizer("PerfectTrackingVirtualizer")
{
	AddParameter(new Parameter<int>("SampleRate", 60));
}

bool PerfectTrackingVirtualizer::CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output)
{
	float animationDuration = trackerHandle.GetAnimationLength();
	int samplerate = dynamic_cast<Parameter<int>*>(parameters["SampleRate"])->GetValue();
	float frameCount = animationDuration * samplerate;


	for (size_t i = 0; i < frameCount + 1; i++)
	{
		float time = i / frameCount;
		time = std::fmin(time, 1.0f);

		Vector3 position = trackerHandle.GetPosition(time);
		Quaternion rotation = trackerHandle.GetRotation(time);

		output.positions.push_back(AnimationCurve::VectorAnimationKey(time * animationDuration, position));
		output.rotations.push_back(AnimationCurve::QuaternionAnimationKey(time * animationDuration, rotation));
	}

	output.scalings.push_back(AnimationCurve::VectorAnimationKey(0.0f, Vector3::one));
	output.scalings.push_back(AnimationCurve::VectorAnimationKey(animationDuration, Vector3::one));

	return true;
}

BaseTrackingVirtualizer* PerfectTrackingVirtualizer::Clone() const
{
	return new PerfectTrackingVirtualizer();
}
