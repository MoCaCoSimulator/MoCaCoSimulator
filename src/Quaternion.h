#pragma once
#include "vector.h"
#include "Matrix.h"

class Quaternion
{
public:
	float x;
	float y;
	float z;
	float w;

	Quaternion() : x(0), y(0), z(0), w(1) {};
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};
	Quaternion(Vector3 angles) { eulerAngles(angles); }
	Quaternion(float x, float y, float z) { eulerAngles(x, y, z); }
	Vector3 eulerAngles() const;
	void eulerAngles(Vector3 angles) { eulerAngles(angles.x, angles.y, angles.z); };
	void eulerAngles(float x, float y, float z);
	Quaternion& angleAxis(const float& angle, const Vector3& axis);
	void ToAngleAxis(float& angle, Vector3& axis) const;
	Matrix toRotationMatrix() const;
	Quaternion normalized() const;
	Quaternion& normalize();
	float dot(const Quaternion& q) const;
	static Quaternion interpolate(const Quaternion& from, const Quaternion& to, float t);
	Quaternion inverse() const;
	Quaternion conjugate() const;
	float magnitudeSqr() const;
	float magnitude() const;

	Quaternion ReverseXW() const { return Quaternion(-this->x, this->y, this->z, -this->w); }

	Vector3 operator*(const Vector3& v) const;
	Matrix operator*(const Matrix& m) const;
	Quaternion operator*(const Quaternion& q) const;
	Quaternion operator*(const float f) const;
	Quaternion operator/(const float f) const;
	Quaternion operator+(const Quaternion& q) const;
	Quaternion operator-(const Quaternion& q) const;
	bool operator==(const Quaternion& q) const;
	bool operator!=(const Quaternion& q) const;

	static Quaternion identity;

	static float Angle(const Quaternion& lhs, const Quaternion& rhs);
	static float Dot(const Quaternion& lhs, const Quaternion& rhs);
	static Quaternion FromToRotation(const Vector3& from, const Vector3& to);
	static Quaternion Lerp(const Quaternion& lhs, const Quaternion& rhs, const float percentage);
	static Quaternion Slerp(const Quaternion& lhs, const Quaternion& rhs, const float percentage);
	static Quaternion SlerpUnclamped(const Quaternion& lhs, const Quaternion& rhs, const float& percentage);
	static Quaternion LookRotation(const Vector3& forward, const Vector3& up);
	static Quaternion LookRotation(const Vector3& forward);
	static Quaternion AngleAxis(const float& angle, const Vector3& axis);
	static Quaternion Inverse(const Quaternion& value);
	static Quaternion Euler(const Vector3& eularAngles);
	static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxAngleDelta);
	static Quaternion Difference(const Quaternion& rot, const Quaternion& target);

	std::string toString() const;
};

