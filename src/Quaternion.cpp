#include "Quaternion.h"

#define QUATERNION_EQUAL_EPSILON 0.001
#define _USE_MATH_DEFINES

#include <math.h> 
#include <sstream>
#include <algorithm>
#include "Utils.h"

Quaternion Quaternion::identity = Quaternion(0, 0, 0, 1);

Vector3 Quaternion::eulerAngles() const
{
	Vector3 angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2.0f * (w * x + y * z);
	double cosr_cosp = 1 - 2 * (x * x + y * y);
	angles.z = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = 2 * (w * y - z * x);
	if (std::abs(sinp) >= 1)
		angles.y = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	else
		angles.y = std::asin(sinp);

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (w * z + x * y);
	double cosy_cosp = 1 - 2 * (y * y + z * z);
	angles.x = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}

//https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
void Quaternion::eulerAngles(float x, float y, float z)
{
	// Abbreviations for the various angular functions
	double cy = cos(z * 0.5);
	double sy = sin(z * 0.5);
	double cp = cos(y * 0.5);
	double sp = sin(y * 0.5);
	double cr = cos(x * 0.5);
	double sr = sin(x * 0.5);

	this->w = cr * cp * cy + sr * sp * sy;
	this->x = sr * cp * cy - cr * sp * sy;
	this->y = cr * sp * cy + sr * cp * sy;
	this->z = cr * cp * sy - sr * sp * cy;
} 

Quaternion& Quaternion::angleAxis(const float& angle, const Vector3& axis)
{
	if (axis.lengthSquared() == 0.0f) 
	{
		*this = Quaternion::identity;
		return *this;
	}

	float f = std::sin(angle / 2);
	x = axis.x * f;
	y = axis.y * f;
	z = axis.z * f;
	w = std::cos(angle / 2);

	normalize();

	return *this;
}

// Source: https://gist.github.com/aeroson/043001ca12fe29ee911e
void Quaternion::ToAngleAxis(float& angle, Vector3& axis) const
{
	Quaternion q = *this;
	if (std::abs(w) > 1.0f)
		q.normalize();

	angle = 2.0f * (float)std::acos(q.w); // angle
	float den = (float)std::sqrt(1.0 - q.w * q.w);
	if (den > 0.0001f)
	{
		axis = Vector3(q.x, q.y, q.z) / den;
	}
	else
	{
		// This occurs when the angle is zero. 
		// Not a problem: just set an arbitrary normalized axis.
		axis = Vector3(1, 0, 0);
	}
}

//https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix
//https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
Matrix Quaternion::toRotationMatrix() const
{
	Quaternion q = normalized();
	return Matrix(
		1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z, 2.0f * q.x * q.y - 2.0f * q.z * q.w, 2.0f * q.x * q.z + 2.0f * q.y * q.w, 0.0f,
		2.0f * q.x * q.y + 2.0f * q.z * q.w, 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z, 2.0f * q.y * q.z - 2.0f * q.x * q.w, 0.0f,
		2.0f * q.x * q.z - 2.0f * q.y * q.w, 2.0f * q.y * q.z + 2.0f * q.x * q.w, 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	/*
	return Matrix(
		(q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z), 2.0f * q.x * q.y - 2.0f * q.w * q.z, 2.0f * q.x * q.z + 2.0f * q.w * q.y, 0.0f,
		2.0f * q.x * q.y + 2.0f * q.w * q.z, q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z, 2.0f * q.y * q.z - 2.0f * q.w * q.x, 0.0f,
		2.0f * q.x * q.z - 2.0f * q.w * q.y, 2.0f * q.y * q.z + 2.0f * q.w * q.x, q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	*/
}

//https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
Vector3 Quaternion::operator*(const Vector3& v) const
{
	Vector3 u(x, y, z);
	float s = w;
	return u * 2.0f * u.dot(v) + v * (s * s - u.dot(u))+ u.cross(v) * 2 * s;
}

Matrix Quaternion::operator*(const Matrix& m) const
{
	return toRotationMatrix() * m;
}

Quaternion Quaternion::operator*(const Quaternion& q) const
{
	return Quaternion(
		w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y + y * q.w + z * q.x - x * q.z,
		w * q.z + z * q.w + x * q.y - y * q.x,
		w * q.w - x * q.x - y * q.y - z * q.z);
}

Quaternion Quaternion::operator*(float s) const
{
	return Quaternion(x * s, y * s, z * s, w * s);
}

Quaternion Quaternion::operator/(float s) const
{
	if (s == 0)
	{
		std::cout << "quaternion division by 0" << std::endl;
		return (*this);
	}
	s = 1 / s;
	return (*this) * s;
}

Quaternion Quaternion::operator+(const Quaternion& q) const
{
	return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
}

Quaternion Quaternion::operator-(const Quaternion& q) const
{
	return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
}

bool Quaternion::operator==(const Quaternion& q) const
{
	return std::abs(this->dot(q) - 1.0f) < QUATERNION_EQUAL_EPSILON;
}

bool Quaternion::operator!=(const Quaternion& q) const
{
	return std::abs(this->dot(q) - 1.0f) > QUATERNION_EQUAL_EPSILON;
}

float Quaternion::Angle(const Quaternion& lhs, const Quaternion& rhs)
{
	/*float dot = Dot(lhs, rhs);
	if (dot > 0.9995f)
		return 0.0f;
	else
		return acos(std::min(std::abs(dot), 1.0f)) * 2.0f;*/
	float f = Quaternion::Dot(lhs, rhs);
	return std::acos(std::min(std::abs(f), 1.0f)) * 2.0f * Utils::RAD2DEG;
}

float Quaternion::Dot(const Quaternion& lhs, const Quaternion& rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

float Quaternion::dot(const Quaternion& q) const
{
	return x * q.x + y * q.y + z * q.z + w * q.w;
}

// Source: https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
Quaternion Quaternion::FromToRotation(const Vector3& from, const Vector3& to)
{
	//return Difference(Matrix().lookAt(from).rotation(), Matrix().lookAt(to).rotation()).normalized();
	//qDebug() << "from" << from.toString().c_str() << "to" << to.toString().c_str();
	//return RotateTowards(LookRotation(from), LookRotation(to), std::numeric_limits<float>::max());
	Vector3 fromNormalized = from.normalized();
	Vector3 toNormalized = to.normalized();

	float s = fromNormalized.dot(toNormalized);
	if (abs(s) > 0.9995f)
		return Quaternion::identity;

	Quaternion q;
	Vector3 a = from.cross(to);
	q.x = a.x;
	q.y = a.y;
	q.z = a.z;
	q.w = std::sqrt(std::pow(from.length(), 2) * std::pow(to.length(), 2)) + from.dot(to);
	return q.normalized();
}

Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, const float t)
{
	float dt = Dot(q1, q2);

	Quaternion q2temp = q2;
	if (dt < 0.0f)
		q2temp = q2temp * -1;

	return (q1 + (q2temp - q1) * t).normalized();
}

Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, const float t)
{
	float dt = Dot(q1, q2);
	Quaternion q2temp = q2;
	if (dt < 0.0f)
	{
		dt = -dt;
		q2temp = q2temp * -1;
	}

	if (dt < 0.9995f)
	{
		float angle = acos(dt);
		float s = 1.0f / sqrt(1.0f - dt * dt); //1.0f / sin(angle)
		float w1 = sin(angle * (1.0f - t)) * s;
		float w2 = sin(angle * t) * s;

		return (q1 * w1 + q2temp * w2).normalized();
	}
	else
	{
		// if the angle is small, use linear interpolation
		return Lerp(q1, q2temp, t);
	}
}

// Source: https://gist.github.com/aeroson/043001ca12fe29ee911e
Quaternion Quaternion::SlerpUnclamped(const Quaternion& lhs, const Quaternion& rhs, const float& percentage)
{
	Quaternion rhsTemp = rhs;

	// if either input is zero, return the other.
	if (lhs.magnitudeSqr() == 0.0f)
	{
		if (rhs.magnitudeSqr() == 0.0f)
		{
			return identity;
		}
		return rhs;
	}
	else if (rhs.magnitudeSqr() == 0.0f)
	{
		return lhs;
	}

	Vector3 lhsVec = Vector3(lhs.x, lhs.y, lhs.z);
	Vector3 rhsVec = Vector3(rhs.x, rhs.y, rhs.z);

	float cosHalfAngle = lhs.w * rhs.w + Vector3::Dot(lhsVec, rhsVec);

	if (cosHalfAngle >= 1.0f || cosHalfAngle <= -1.0f)
	{
		// angle = 0.0f, so just return one input.
		return lhs;
	}
	else if (cosHalfAngle < 0.0f)
	{
		rhsTemp.x = -rhs.x;
		rhsTemp.y = -rhs.y;
		rhsTemp.z = -rhs.z;
		rhsTemp.w   = -rhs.w;
		cosHalfAngle = -cosHalfAngle;
	}

	float blendA;
	float blendB;
	if (cosHalfAngle < 0.99f)
	{
		// do proper slerp for big angles
		float halfAngle = (float)std::acos(cosHalfAngle);
		float sinHalfAngle = (float)std::sin(halfAngle);
		float oneOverSinHalfAngle = 1.0f / sinHalfAngle;
		blendA = (float)std::sin(halfAngle * (1.0f - percentage)) * oneOverSinHalfAngle;
		blendB = (float)std::sin(halfAngle * percentage) * oneOverSinHalfAngle;
	}
	else
	{
		// do lerp if angle is really small.
		blendA = 1.0f - percentage;
		blendB = percentage;
	}

	float x = blendA * lhs.x + blendB * rhs.x;
	float y = blendA * lhs.y + blendB * rhs.y;
	float z = blendA * lhs.z + blendB * rhs.z;
	float w = blendA * lhs.w + blendB * rhs.w;
	Quaternion result = Quaternion(x,y,z,w);
	if (result.magnitudeSqr() > 0.0f)
		return result.normalized();
	else
		return identity;
}

Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up)
{
	Matrix rotationMatrix = Matrix::FromRotationAxis(forward, up);
	return rotationMatrix.rotation();
}

Quaternion Quaternion::LookRotation(const Vector3& forward)
{
	Matrix rotationMatrix = Matrix().lookAt(forward);
	return rotationMatrix.rotation();
}

// https://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/index.htm
Quaternion Quaternion::AngleAxis(const float& angle, const Vector3& axis)
{
	if (axis.lengthSquared() == 0.0f)
		return Quaternion::identity;

	Quaternion q = Quaternion();
	q.angleAxis(angle, axis.normalized());
	return q;
}

Quaternion Quaternion::Inverse(const Quaternion& value)
{
	return value.inverse();
}

Quaternion Quaternion::Euler(const Vector3& eulerAngles)
{
	return Quaternion(eulerAngles);
}

Quaternion Quaternion::RotateTowards(Quaternion from, Quaternion to, float maxAngleDelta)
{
	float angle = Quaternion::Angle(from, to);
	qDebug() << "ROTATE TOWARDS" << from.eulerAngles().toString().c_str() << "to" << to.eulerAngles().toString().c_str() << "=" << angle;
	if (angle == 0.0f) 
		return to;
	return SlerpUnclamped(from, to, std::min(1.0f, maxAngleDelta / angle));
}

Quaternion Quaternion::Difference(const Quaternion& rot, const Quaternion& target)
{
	Quaternion rotInverse = rot.inverse();
	return target * rotInverse;
}

std::string Quaternion::toString() const
{
	std::ostringstream oss;
	oss << "X: " << x << " Y: " << y << " Z: " << z << " W: " << w;
	return oss.str();
}

Quaternion Quaternion::interpolate(const Quaternion& from, const Quaternion& to, float t)
{
	return Slerp(from, to, t);
}

Quaternion Quaternion::inverse() const
{
	return conjugate() / magnitudeSqr();
}

Quaternion Quaternion::conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

float Quaternion::magnitudeSqr() const
{
	return (x * x + y * y + z * z + w * w);
}

float Quaternion::magnitude() const
{
	return sqrt(magnitudeSqr());
}

Quaternion Quaternion::normalized() const
{
	const float n = 1.0f / magnitude();
	return (*this) * n;
}

Quaternion& Quaternion::normalize()
{
	const float n = 1.0f / magnitude();
	*this = *this * n;
	return *this;
}