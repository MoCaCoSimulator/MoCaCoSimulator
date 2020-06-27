#pragma once

#include <assimp\anim.h>
#include <vector>
#include "vector.h"
#include "Quaternion.h"
#include <cassert>

struct AnimationCurve
{
#pragma region Internal_Datastructures

	struct AnimationKey
	{
		AnimationKey() : time(0) {}
		float time; //in seconds

		static float GetInterpolationTime(const AnimationCurve::AnimationKey& from, const AnimationCurve::AnimationKey& to, const float time)
		{
			return (time - from.time) / (to.time - from.time);
		}
	};

	struct VectorAnimationKey : public AnimationKey
	{
		Vector3 value;

		// Internal constructor
		VectorAnimationKey() : value(Vector3::zero) {}

		// Internal constructor
		VectorAnimationKey(float t, Vector3 v) : value(v) { time = t; }

		// Assimp constructor
		VectorAnimationKey(aiVectorKey aiAnimationKey)
		{
			time = aiAnimationKey.mTime;
			value = Vector3(aiAnimationKey.mValue.x, aiAnimationKey.mValue.y, aiAnimationKey.mValue.z);
		}

		static Vector3 Interpolate(const float& time, const std::vector<AnimationCurve::VectorAnimationKey>& vectorKey)
		{
			if (vectorKey.size() == 1)
				return (*vectorKey.begin()).value;

			int keyIndex = FindActiveVectorKeyIndex(time, vectorKey);
			AnimationCurve::VectorAnimationKey from = vectorKey[keyIndex];

			if (vectorKey.size() == keyIndex + 1)
				return from.value;

			AnimationCurve::VectorAnimationKey to = vectorKey[keyIndex + 1];
			float t = GetInterpolationTime(from, to, time);

			return Vector3::interpolate(from.value, to.value, t);
		}

		static int FindActiveVectorKeyIndex(const float& time, const std::vector<AnimationCurve::VectorAnimationKey>& keys)
		{
			for (int i = keys.size() - 2; i >= 0; i--)
			{
				if (time >= keys[i].time)
					return i;
			}
			return -1;
		}
	};

	struct QuaternionAnimationKey : public AnimationKey
	{
		Quaternion value;
		// Internal constructor
		QuaternionAnimationKey() : value(Quaternion::identity) {}

		// Internal constructor
		QuaternionAnimationKey(float t, Quaternion v) : value(v) { time = t; }

		// Assimp constructor
		QuaternionAnimationKey(aiQuatKey aiAnimationKey)
		{
			time = aiAnimationKey.mTime;
			value = Quaternion(aiAnimationKey.mValue.x, aiAnimationKey.mValue.y, aiAnimationKey.mValue.z, aiAnimationKey.mValue.w);
		}

		static Quaternion Interpolate(float time, const std::vector<AnimationCurve::QuaternionAnimationKey>& quatKey)
		{
			if (quatKey.size() == 1)
				return (*quatKey.begin()).value;

			int keyIndex = FindActiveQuaternionKeyIndex(time, quatKey);
			AnimationCurve::QuaternionAnimationKey from = quatKey[keyIndex];

			if (quatKey.size() == keyIndex + 1)
				return from.value;

			AnimationCurve::QuaternionAnimationKey to = quatKey[keyIndex + 1];
			float t = GetInterpolationTime(from, to, time);

			return Quaternion::interpolate(from.value, to.value, t);
		}

		static int FindActiveQuaternionKeyIndex(float time, const std::vector<AnimationCurve::QuaternionAnimationKey>& keys)
		{
			for (int i = keys.size() - 2; i >= 0; i--)
			{
				if (time >= keys[i].time)
					return i;
			}
			return -1;
		}
	};

#pragma endregion Internal_Datastructures

	AnimationCurve() : name("") {}
	AnimationCurve(aiNodeAnim* aiNodeAnim);

	Vector3 GetPosition(const float& time) const { return VectorAnimationKey::Interpolate(time, positions); }
	Quaternion GetRotation(const float& time) const { return QuaternionAnimationKey::Interpolate(time, rotations); }
	Vector3 GetScale(const float& time) const { return VectorAnimationKey::Interpolate(time, scalings); }

	std::string name;
	std::vector<VectorAnimationKey> positions;
	std::vector<QuaternionAnimationKey> rotations;
	std::vector<VectorAnimationKey> scalings;
};