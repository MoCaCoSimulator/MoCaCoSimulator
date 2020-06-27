#include "FloatCurve.h"
#include <cmath>

namespace RootMotion
{
	float FloatCurve::Evaluate(float time) const
	{
		int fromIndex = FindActiveKeyIndex(time);
		if (fromIndex < 0)
			return 0.0f;

		int toIndex = fromIndex + 1;
		const Keyframe* from = &keys[fromIndex];
		const Keyframe* to = &keys[fromIndex];
		if (toIndex < keys.size())
			to = &keys[toIndex];

		float t = GetInterpolationTime(*from, *to, time);
		return from->value + t * (to->value - from->value);
	};

	float FloatCurve::GetInterpolationTime(const Keyframe& from, const Keyframe& to, const float& time)
	{
		if (from.time == to.time)
			return 1.0f;
		return (time - from.time) / (to.time - from.time);
	}

	int FloatCurve::FindActiveKeyIndex(float time) const
	{
		for (int i = keys.size() - 2; i >= 0; i--)
		{
			if (time >= keys[i].time)
				return i;
		}
		return -1;
	}

	bool FloatCurve::operator==(const FloatCurve& rhs) 
	{
		return this == &rhs; 
	};
}