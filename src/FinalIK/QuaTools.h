#pragma once

#include "../Quaternion.h"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include "../Utils.h"

namespace RootMotion
{
	/// <summary>
	/// Helper methods for dealing with Quaternions.
	/// </summary>
	static class QuaTools
	{
	public:
		/// <summary>
		/// Optimized Quaternion.Lerp
		/// </summary>
		static Quaternion Lerp(Quaternion fromRotation, Quaternion toRotation, float weight)
		{
			if (weight <= 0.0f) return fromRotation;
			if (weight >= 1.0f) return toRotation;

			return Quaternion::Lerp(fromRotation, toRotation, weight);
		}

		/// <summary>
		/// Optimized Quaternion.Slerp
		/// </summary>
		static Quaternion Slerp(Quaternion fromRotation, Quaternion toRotation, float weight)
		{
			if (weight <= 0.0f) return fromRotation;
			if (weight >= 1.0f) return toRotation;

			return Quaternion::Slerp(fromRotation, toRotation, weight);
		}

		/// <summary>
		/// Returns the rotation from identity Quaternion to "q", interpolated linearily by "weight".
		/// </summary>
		static Quaternion LinearBlend(Quaternion q, float weight)
		{
			if (weight <= 0.0f) return Quaternion::identity;
			if (weight >= 1.0f) return q;
			return Quaternion::Lerp(Quaternion::identity, q, weight);
		}

		/// <summary>
		/// Returns the rotation from identity Quaternion to "q", interpolated spherically by "weight".
		/// </summary>
		static Quaternion SphericalBlend(Quaternion q, float weight)
		{
			if (weight <= 0.0f) return Quaternion::identity;
			if (weight >= 1.0f) return q;
			return Quaternion::Slerp(Quaternion::identity, q, weight);
		}

		/// <summary>
		/// Creates a FromToRotation, but makes sure it's axis remains fixed near to the Quaternion singularity point.
		/// </summary>
		/// <returns>
		/// The from to rotation around an axis.
		/// </returns>
		/// <param name='fromDirection'>
		/// From direction.
		/// </param>
		/// <param name='toDirection'>
		/// To direction.
		/// </param>
		/// <param name='axis'>
		/// Axis. Should be normalized before passing into this method.
		/// </param>
		static Quaternion FromToAroundAxis(Vector3 fromDirection, Vector3 toDirection, Vector3 axis)
		{
			Quaternion fromTo = Quaternion::FromToRotation(fromDirection, toDirection);

			float angle = 0;
			Vector3 freeAxis = Vector3::zero;

			fromTo.ToAngleAxis(angle, freeAxis);
			angle *= Utils::RAD2DEG;

			float dot = freeAxis.dot(axis);
			if (dot < 0) angle = -angle;

			qDebug() << "fromtoaround" << angle;

			return Quaternion::AngleAxis(angle * Utils::DEG2RAD, axis);
		}

		/// <summary>
		/// Gets the rotation that can be used to convert a rotation from one axis space to another.
		/// </summary>
		static Quaternion RotationToLocalSpace(Quaternion space, Quaternion rotation)
		{
			return (space.inverse() * rotation).inverse();
		}

		/// <summary>
		/// Gets the Quaternion from rotation "from" to rotation "to".
		/// </summary>
		static Quaternion FromToRotation(Quaternion from, Quaternion to)
		{
			if (to == from) return Quaternion::identity;

			return to * from.inverse();
		}


		/// <summary>
		/// Gets the closest direction axis to a vector. Input vector must be normalized!
		/// </summary>
		static Vector3 GetAxis(Vector3 v)
		{
			Vector3 closest = Vector3::right;
			bool neg = false;

			float x = v.dot(Vector3::right);
			float maxAbsDot = abs(x);
			if (x < 0.0f) neg = true;

			float y = v.dot(Vector3::up);
			float absDot = abs(y);
			if (absDot > maxAbsDot)
			{
				maxAbsDot = absDot;
				closest = Vector3::up;
				neg = y < 0.0f;
			}

			float z = v.dot(Vector3::forward);
			absDot = abs(z);
			if (absDot > maxAbsDot)
			{
				closest = Vector3::forward;
				neg = z < 0.0f;
			}

			if (neg) closest = -closest;
			return closest;
		}

		/// <summary>
		/// Clamps the rotation similar to V3Tools.ClampDirection.
		/// </summary>
		static Quaternion ClampRotation(Quaternion rotation, float clampWeight, int clampSmoothing)
		{
			if (clampWeight >= 1.0f) return Quaternion::identity;
			if (clampWeight <= 0.0f) return rotation;

			float angle = Quaternion::Angle(Quaternion::identity, rotation);
			float dot = 1.0f - (angle / 180.0f);
			float targetClampMlp = std::clamp(1.0f - ((clampWeight - dot) / (1.0f - dot)), 0.0f, 1.0f);
			float clampMlp = std::clamp(dot / clampWeight, 0.0f, 1.0f);

			// Sine smoothing iterations
			for (int i = 0; i < clampSmoothing; i++)
			{
				float sinF = clampMlp * M_PI * 0.5f;
				clampMlp = std::sin(sinF);
			}

			return Quaternion::Slerp(Quaternion::identity, rotation, clampMlp * targetClampMlp);
		}

		/// <summary>
		/// Clamps an angular value.
		/// </summary>
		static float ClampAngle(float angle, float clampWeight, int clampSmoothing)
		{
			if (clampWeight >= 1.0f) return 0.0f;
			if (clampWeight <= 0.0f) return angle;

			float dot = 1.0f - (abs(angle) / 180.0f);
			float targetClampMlp = std::clamp(1.0f - ((clampWeight - dot) / (1.0f - dot)), 0.0f, 1.0f);
			float clampMlp = std::clamp(dot / clampWeight, 0.0f, 1.0f);

			// Sine smoothing iterations
			for (int i = 0; i < clampSmoothing; i++)
			{
				float sinF = clampMlp * M_PI * 0.5f;
				clampMlp = std::sin(sinF);
			}

			return angle * clampMlp * targetClampMlp; // Was lerp
		}

		/// <summary>
		/// Used for matching the rotations of objects that have different orientations.
		/// </summary>
		static Quaternion MatchRotation(Quaternion targetRotation, Vector3 targetforwardAxis, Vector3 targetUpAxis, Vector3 forwardAxis, Vector3 upAxis)
		{
			Quaternion f = Quaternion::LookRotation(forwardAxis, upAxis);
			Quaternion fTarget = Quaternion::LookRotation(targetforwardAxis, targetUpAxis);

			Quaternion d = targetRotation * fTarget;
			return d * f.inverse();
		}

		/// <summary>
		/// Converts an Euler rotation from 0 to 360 representation to -180 to 180.
		/// </summary>
		static Vector3 ToBiPolar(Vector3 euler)
		{
			return Vector3(ToBiPolar(euler.x), ToBiPolar(euler.y), ToBiPolar(euler.z));
		}

		/// <summary>
		/// Converts an angular value from 0 to 360 representation to -180 to 180.
		/// </summary>
		static float ToBiPolar(float angle)
		{
			angle = fmod(angle, 360.0f);
			if (angle >= 180.0f) return angle - 360.0f;
			if (angle <= -180.0f) return angle + 360.0f;
			return angle;
		}
	};
}