#define _USE_MATH_DEFINES

#include "vector.h"
#include <assert.h>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "Utils.h"
#include <qdebug.h>

#define EPSILON 1e-5
//#define DEG2RAD(angleDegrees) ((angleDegrees) * M_PI / 180.0)
//#define RAD2DEG(angleRadians) ((angleRadians) * 180.0 / M_PI)

Vector2 Vector2::zero = Vector2(0, 0);
Vector2 Vector2::one = Vector2(1, 1);
Vector2 Vector2::left = Vector2(1, 0);
Vector2 Vector2::right = Vector2(-1, 0);
Vector2 Vector2::top = Vector2(0, 1);
Vector2 Vector2::bottom = Vector2(0, -1);

float Vector2::dot(const Vector2& v) const
{
	return this->x * v.x + this->y * v.y;
}

Vector2 Vector2::cross(const Vector2& v) const
{
	Vector2 vec(this->y * v.x - this->x * v.y, this->x * v.y - this->x * v.y);
	return vec;
}

Vector2 Vector2::operator+(const Vector2& v) const
{
	Vector2 vec(this->x + v.x, this->y + v.y);
	return vec;
}

Vector2 Vector2::operator-(const Vector2& v) const
{
	Vector2 vec(this->x - v.x, this->y - v.y);
	return vec;
}

Vector2& Vector2::operator=(const Vector2& v)
{
	this->x = v.x;
	this->y = v.y;
	return *this;
}

Vector2 Vector2::operator*(float c) const
{
	Vector2 vec(this->x * c, this->y * c);
	return vec;
}

Vector2 Vector2::operator-() const
{
	Vector2 vec(this->x * (-1), this->y * (-1));
	return vec;
}

Vector2& Vector2::operator+=(const Vector2& v)
{
	*this = *this + v;
	return *this;
}

Vector2& Vector2::normalize()
{
	float length = this->length();
	if (length < EPSILON || length == 1)
		return *this;

	this->x = this->x / length;
	this->y = this->y / length;
	return *this;
}

float Vector2::length() const
{
	return sqrt(this->lengthSquared());
}

float Vector2::lengthSquared() const
{
	return this->x * this->x + this->y * this->y;
}

void Vector2::print() const
{
	printf("X: %f Y: %f\n", this->x, this->y);
}

Vector2 Vector2::interpolate(const Vector2& from, const Vector2& to, float t)
{
	t = std::fmin(std::fmax(t, 0.f), 1.f);
	return (from * (1 - t) + to * t);
}

const Vector3 Vector3::forward = Vector3(0, 0, 1);
const Vector3 Vector3::back = Vector3(0, 0, -1);
const Vector3 Vector3::up = Vector3(0, 1, 0);
const Vector3 Vector3::down = Vector3(0, -1, 0);
const Vector3 Vector3::right = Vector3(1, 0, 0);
const Vector3 Vector3::left = Vector3(-1, 0, 0);
const Vector3 Vector3::zero = Vector3(0, 0, 0);
const Vector3 Vector3::one = Vector3(1, 1, 1);

float Vector3::dot(const Vector3& v) const
{
	return this->x * v.x + this->y * v.y + this->z * v.z;
}

Vector3 Vector3::cross(const Vector3& v) const
{
	Vector3 vec(this->y * v.z - this->z * v.y, this->z * v.x - this->x * v.z, this->x * v.y - this->y * v.x);
	return vec;
}

Vector3 Vector3::operator+(const Vector3& v) const
{
	Vector3 vec(this->x + v.x, this->y + v.y, this->z + v.z);
	return vec;
}

Vector3 Vector3::operator-(const Vector3& v) const
{
	Vector3 vec(this->x - v.x, this->y - v.y, this->z - v.z);
	return vec;
}

bool Vector3::operator==(const Vector3& v)
{
	if (this->x == v.x && this->y == v.y && this->z == v.z)
		return true;
	else
		return false;
}

bool Vector3::operator!=(const Vector3& v)
{
	if (this->x != v.x || this->y != v.y || this->z != v.z)
		return true;
	else
		return false;
}

Vector3& Vector3::operator=(const Vector3& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	return *this;
}

Vector3 Vector3::operator*(float c) const
{
	Vector3 vec(this->x * c, this->y * c, this->z * c);
	return vec;
}

Vector3 Vector3::operator/(float divider)
{
	Vector3 vec(this->x / divider, this->y / divider, this->z / divider);
	return vec;
}

Vector3 Vector3::operator-() const
{
	Vector3 vec(this->x * (-1), this->y * (-1), this->z * (-1));
	return vec;
}

Vector3& Vector3::multiplyElements(const Vector3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

Vector3& Vector3::operator+=(const Vector3& v)
{
	*this = *this + v;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& v)
{
	*this = *this - v;
	return *this;
}

Vector3& Vector3::normalize()
{
	float length = this->length();

	if (length == 0.0f) {
		*this = Vector3::zero;
		return *this;
	}

	if (abs(length-1.0f) < 0.0005f)//== 1.0f)
		return *this;

	this->x = this->x / length;
	this->y = this->y / length;
	this->z = this->z / length;
	return *this;
}

Vector3 Vector3::normalized() const
{
	return Vector3(*this).normalize();
}

float Vector3::length() const
{
	return sqrt(this->lengthSquared());
}

float Vector3::lengthSquared() const
{
	return this->x*this->x + this->y*this->y + this->z*this->z;
}

Vector3 Vector3::reflection(const Vector3& normal) const
{
	return *this - normal * (2 * this->dot(normal));
}

Vector3 Vector3::refraction(const Vector3& normal, float fromRefractivity, float toRefractivity) const
{
	float cosl = normal.dot(*this);

	float rate = fromRefractivity / toRefractivity;
	float sinT_para = rate * rate * (1 - cosl * cosl);
	//float sinT2 = (fromRefractivity*fromRefractivity*(1 - cosI * cosI)) / (toRefractivity * toRefractivity);

	if (sinT_para > 1.f)
		return this->reflection(normal);

	Vector3 T_para = (normal * cosl - *this) * rate;
	Vector3 T_orto = normal * sqrt(1.f - sinT_para);
	Vector3 T = T_para - T_orto;
	//Vector T = *this * rate - normal * (rate + sqrt(1.f - sinT_para));

	return T.normalize();
}

Vector3 Vector3::upFromForward(const Vector3 forward) {
	//Calc right vector from crossing global up vector
	Vector3 right = forward.cross(Vector3::up).normalize();
	//Calc up vector from crossing right with forward Vector
	return right.cross(forward).normalize();
}

std::string Vector3::toString() const
{
	std::ostringstream oss;
	oss << "X: " << this->x << " Y: " << this->y << " Z: " << this->z;
	return oss.str();
}

Vector3 Vector3::interpolate(const Vector3& from, const Vector3& to, float t)
{
	t = std::fmin(std::fmax(t, 0.f), 1.f);
	return Lerp(from, to, t);
}

float Vector3::Distance(const Vector3 lhs, const Vector3 rhs)
{
	return (lhs - rhs).length();;
}

Vector3 Vector3::ProjectOnPlane(Vector3 vector, Vector3 planeNormal)
{
	float sqrMag = planeNormal.dot(planeNormal);
	if (sqrMag < EPSILON)
		return vector;
	else
	{
		float dot = vector.dot(planeNormal);
		return Vector3(vector.x - planeNormal.x * dot / sqrMag,
					   vector.y - planeNormal.y * dot / sqrMag,
					   vector.z - planeNormal.z * dot / sqrMag);
	}
}

float Vector3::SignedAngleDegree(Vector3 from, Vector3 to, Vector3 axis)
{
	float unsignedAngle = AngleDegree(from, to);

	float cross_x = from.y * to.z - from.z * to.y;
	float cross_y = from.z * to.x - from.x * to.z;
	float cross_z = from.x * to.y - from.y * to.x;
	int sign = Utils::Sign(axis.x * cross_x + axis.y * cross_y + axis.z * cross_z);
	return unsignedAngle * sign;
}

float Vector3::AngleDegree(Vector3 from, Vector3 to)
{
	// sqrt(a) * sqrt(b) = sqrt(a * b) -- valid for real numbers
	float denominator = (float)std::sqrt(from.lengthSquared() * to.lengthSquared());
	if (denominator < 1e-15f)
		return 0.0f;

	float dot = std::clamp(from.dot(to) / denominator, -1.0f, 1.0f);
	return ((float)std::acos(dot)) * Utils::RAD2DEG;
}

Vector3 Vector3::Lerp(const Vector3& from, const Vector3& to, const float t)
{
	return from + (to - from) * t;
}

// Source: https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
// NOTE: Added linear magnitude interpolation
Vector3 Vector3::Slerp(const Vector3& from, const Vector3& to, const float t)
{
	Vector3 fromNormalized = from.normalized();
	Vector3 toNormalized = to.normalized();
	float dot = fromNormalized.dot(toNormalized);

	// if the angle is small, use linear interpolation
	if (abs(dot) > 0.9995f)
		return Lerp(from, to, t);

	float theta = std::acos(dot) * t;
	Vector3 relativeVec = toNormalized - fromNormalized * dot;
	relativeVec = relativeVec.normalized();

	Vector3 normalized = (fromNormalized * std::cos(theta)) + (relativeVec * std::sin(theta));
	float length = Utils::Lerp(from.length(), to.length(), t);
	return normalized * length;
}

Vector3 Vector3::ClampMagnitude(const Vector3& toClamp, const float value)
{
	if (value == 0)
		return Vector3::zero;

	float magnitude = toClamp.length();
	if (magnitude <= value)
		return toClamp;

	return toClamp.normalized() * value;
}

Vector3 Vector3::Project(const Vector3& lhs, const Vector3& rhs)
{
	float sqrMag = rhs.dot(rhs);
	if (sqrMag < std::numeric_limits<float>::epsilon())
		return zero;
	else
	{
		float dot = lhs.dot(rhs);
		return Vector3(rhs.x * dot / sqrMag, rhs.y * dot / sqrMag, rhs.z * dot / sqrMag);
	}
}

void Vector3::OrthoNormalize(Vector3& normal, Vector3& tangent)
{
	normal = normal.normalize();
	tangent = tangent.normalize();
	tangent = tangent.cross(normal);
}

Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
{
	return lhs.cross(rhs);
}

float Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
{
	return lhs.dot(rhs);
}

float Vector3::SqrMagnitude(const Vector3& value)
{
	return value.lengthSquared();
}

float Vector3::Magnitude(const Vector3& value)
{
	return value.length();
}

bool Vector3::triangleIntersection(const Vector3& d, const Vector3& a, const Vector3& b, const Vector3& c, float& s) const
{
	Vector3 n = ((b - a).cross(c - a)).normalize();
	s = ( n.dot(a) - n.dot(*this) ) / n.dot(d);
	if (s < 0)
		return false;

	Vector3 p = *this + d * s;

	//abc.length();
	float abc = ((b - a).cross(c - a)).length();
	float abp = ((b - a).cross(p - a)).length();
	float acp = ((c - a).cross(p - a)).length();
	float bcp = ((c - b).cross(p - b)).length();

	if ( abc + EPSILON >= abp + acp + bcp )
		return true;

	return false;
}

bool Vector3::planeIntersection(const Vector3& d, const Vector3& pos, const Vector3& n, Vector3& s) const
{
	float scale = (n.dot(pos) - n.dot(*this)) / n.dot(d);
	if (scale < 0)
		return false;

	s = *this + d * scale;
	return true;
}

Vector4 Vector4::zero = Vector4(0, 0, 0, 0);
Vector4 Vector4::one = Vector4(1, 1, 1, 0);

float Vector4::dot(const Vector4& v) const
{
	return this->x * v.x + this->y * v.y + this->z * v.z + this->w * v.w;
}

Vector4 Vector4::cross(const Vector4& v) const
{
	Vector4 vec(this->y * v.z - this->z * v.y, this->z * v.w - this->w * v.z, this->w * v.x - this->x * v.w, this->x * v.y - this->y * v.x);
	return vec;
}

Vector4 Vector4::operator+(const Vector4& v) const
{
	Vector4 vec(this->x + v.x, this->y + v.y, this->z + v.z, this->w + v.w);
	return vec;
}

Vector4 Vector4::operator-(const Vector4& v) const
{
	Vector4 vec(this->x - v.x, this->y - v.y, this->z - v.z, this->w - v.w);
	return vec;
}

Vector4& Vector4::operator=(const Vector4& v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
	this->w = v.w;
	return *this;
}

Vector4 Vector4::operator*(float c) const
{
	Vector4 vec(this->x * c, this->y * c, this->z * c, this->w * c);
	return vec;
}

Vector4 Vector4::operator-() const
{
	Vector4 vec(this->x * (-1), this->y * (-1), this->z * (-1), this->w * (-1));
	return vec;
}

Vector4& Vector4::operator+=(const Vector4& v)
{
	*this = *this + v;
	return *this;
}

Vector4& Vector4::normalize()
{
	float length = this->length();
	if (length < EPSILON || length == 1)
		return *this;

	this->x = this->x / length;
	this->y = this->y / length;
	this->z = this->z / length;
	this->w = this->w / length;
	return *this;
}

float Vector4::length() const
{
	return sqrt(this->lengthSquared());
}

float Vector4::lengthSquared() const
{
	return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;
}

void Vector4::print() const
{
	printf("X: %f Y: %f Z: %f W: %f\n", this->x, this->y, this->z, this->w);
}

Vector4 Vector4::interpolate(const Vector4& from, const Vector4& to, float t)
{
	t = std::fmin(std::fmax(t, 0.f), 1.f);
	return (from * (1 - t) + to * t);
}
