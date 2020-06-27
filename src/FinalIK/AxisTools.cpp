#include "AxisTools.h"

namespace RootMotion
{
	Vector3 AxisTools::ToVector3(Axis axis)
	{
		if (axis == Axis::X) return Vector3::right;
		if (axis == Axis::Y) return Vector3::up;
		return Vector3::forward;
	}

	Axis AxisTools::ToAxis(Vector3 v)
	{
		float absX = std::abs(v.x);
		float absY = std::abs(v.y);
		float absZ = std::abs(v.z);

		Axis d = Axis::X;
		if (absY > absX && absY > absZ) d = Axis::Y;
		if (absZ > absX && absZ > absY) d = Axis::Z;
		return d;
	}

	Axis AxisTools::GetAxisToPoint(const Transform* t, Vector3 worldPosition)
	{
		Vector3 axis = GetAxisVectorToPoint(t, worldPosition);
		if (axis == Vector3::right) return Axis::X;
		if (axis == Vector3::up) return Axis::Y;
		return Axis::Z;
	}

	Axis AxisTools::GetAxisToDirection(const Transform* t, Vector3 direction)
	{
		Vector3 axis = GetAxisVectorToDirection(t, direction);
		if (axis == Vector3::right) return Axis::X;
		if (axis == Vector3::up)    return Axis::Y;
		return Axis::Z;
	}

	Vector3 AxisTools::GetAxisVectorToPoint(const Transform* t, Vector3 worldPosition)
	{
		return GetAxisVectorToDirection(t, worldPosition - t->Position());
	}

	Vector3 AxisTools::GetAxisVectorToDirection(const Transform* t, Vector3 direction)
	{
		return GetAxisVectorToDirection(t->Rotation(), direction);
	}

	Vector3 AxisTools::GetAxisVectorToDirection(Quaternion r, Vector3 direction)
	{
		direction = direction.normalized();
		Vector3 axis = Vector3::right;

		float dotX = std::abs(Vector3::Dot((r * Vector3::right).normalized(), direction));
		float dotY = std::abs(Vector3::Dot((r * Vector3::up).normalized(), direction));
		if (dotY > dotX) axis = Vector3::up;
		float dotZ = std::abs(Vector3::Dot((r * Vector3::forward).normalized(), direction));
		if (dotZ > dotX && dotZ > dotY) axis = Vector3::forward;

		return axis;
	}
}
