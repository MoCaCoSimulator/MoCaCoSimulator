#ifndef __SimpleRayTracer__vector__
#define __SimpleRayTracer__vector__

#include <iostream>
#include <qdebug.h>

class Vector2
{
public:
	float x, y;

	Vector2(float x, float y) :x(x), y(y) {};
	Vector2() : x(0), y(0) {};

	float dot(const Vector2& v) const;
	Vector2 cross(const Vector2& v) const;
	Vector2 operator+(const Vector2& v) const;
	Vector2 operator-(const Vector2& v) const;
	Vector2& operator=(const Vector2& v);
	Vector2& operator+=(const Vector2& v);
	Vector2 operator*(float c) const;
	Vector2 operator-() const;
	Vector2& normalize();
	float length() const;
	float lengthSquared() const;
	void print() const;
	static Vector2 interpolate(const Vector2& from, const Vector2& to, float t);

	static Vector2 zero;
	static Vector2 one;
	static Vector2 left;
	static Vector2 right;
	static Vector2 top;
	static Vector2 bottom;
};

class Vector3
{
public:
    float x;
    float y;
    float z;
    
	Vector3(float x, float y, float z) :x(x), y(y), z(z) {};
	Vector3() : x(0), y(0), z(0) {};
	Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {};
    
    float dot(const Vector3& v) const;
    Vector3 cross(const Vector3& v) const;
    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
	bool operator==(const Vector3& v);
	bool operator!=(const Vector3& v);
	Vector3& operator=(const Vector3& v);
	Vector3& operator+=(const Vector3& v);
	Vector3& operator-=(const Vector3& v);
	Vector3 operator*(float c) const;
	Vector3 operator/(float divider);
    Vector3 operator-() const;
	Vector3& multiplyElements(const Vector3& v);
	Vector3& normalize();
	Vector3 normalized() const;
    float length() const;
    float lengthSquared() const;
    Vector3 reflection( const Vector3& normal) const;
	static Vector3 upFromForward(const Vector3 forward);
	Vector3 refraction(const Vector3& normal, float fromRefractivity, float toRefractivity) const;
    bool triangleIntersection( const Vector3& d, const Vector3& a, const Vector3& b,
                              const Vector3& c, float& s) const;
	bool planeIntersection(const Vector3& d, const Vector3& pos, const Vector3& n, Vector3& s) const;
	std::string toString() const;
	static Vector3 interpolate(const Vector3& from, const Vector3& to, float t);

	Vector3 ReverseX() const { return Vector3(-this->x, this->y, this->z); }

	const static Vector3 forward;
	const static Vector3 back;
	const static Vector3 up;
	const static Vector3 down;
	const static Vector3 right;
	const static Vector3 left;
	const static Vector3 zero;
	const static Vector3 one;

	static float Distance(const Vector3 lhs, const Vector3 rhs);
	static Vector3 ProjectOnPlane(Vector3 vector, Vector3 planeNormal);
	static float SignedAngleDegree(Vector3 from, Vector3 to, Vector3 axis);
	static float AngleDegree(Vector3 from, Vector3 to);
	static Vector3 Lerp(const Vector3& lhs, const Vector3& rhs, const float value);
	static Vector3 Slerp(const Vector3& lhs, const Vector3& rhs, const float value);
	static Vector3 ClampMagnitude(const Vector3& toClamp, const float value);
	static Vector3 Project(const Vector3& lhs, const Vector3& rhs);
	static void OrthoNormalize(Vector3& normal, Vector3& tangent);
	static Vector3 Cross(const Vector3& lhs, const Vector3& rhs);
	static float Dot(const Vector3& lhs, const Vector3& rhs);
	static float SqrMagnitude(const Vector3& value);
	static float Magnitude(const Vector3& value);
 };

class Vector4
{
public:
	float x, y, z, w;

	Vector4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {};
	Vector4() : x(0), y(0), z(0), w(0) {};

	float dot(const Vector4& v) const;
	Vector4 cross(const Vector4& v) const;
	Vector4 operator+(const Vector4& v) const;
	Vector4 operator-(const Vector4& v) const;
	Vector4& operator=(const Vector4& v);
	Vector4& operator+=(const Vector4& v);
	Vector4 operator*(float c) const;
	Vector4 operator-() const;
	Vector4& normalize();
	float length() const;
	float lengthSquared() const;
	void print() const;
	static Vector4 interpolate(const Vector4& from, const Vector4& to, float t);

	static Vector4 zero;
	static Vector4 one;
};

#endif /* defined(__SimpleRayTracer__vector__) */
