
#include <qdebug.h>
#ifndef __RealtimeRending__Matrix__
#define __RealtimeRending__Matrix__

#include <iostream>
#include <sstream>
#include "vector.h"

class Quaternion;

class Matrix
{
public:
    union
    {
        struct {
            float m00, m10,m20,m30;
            float m01, m11,m21,m31;
            float m02, m12,m22,m32;
            float m03, m13,m23,m33;
        };
        struct { float m[16]; };
        struct { float mMul[4][4]; };
    };
    Matrix();
    Matrix( float _00, float _01, float _02, float _03,
            float _10, float _11, float _12, float _13,
            float _20, float _21, float _22, float _23,
            float _30, float _31, float _32, float _33 );
	Matrix(const Matrix& other);
    
    operator float*();
    operator const float* const();
    
    Matrix operator*(const Matrix& M) const;
    Matrix& operator*=(const Matrix& M);
	Vector3 operator*(const Vector3& v) const;
	Vector4 operator*(const Vector4& v) const;
	Quaternion operator*(const Quaternion& q) const;
	Matrix operator*(float f) const;
	Matrix& operator*=(float f);
	Matrix operator+(const Matrix& M) const;
	Matrix& operator+=(const Matrix& M);
	Matrix operator-(const Matrix& M) const;
	Matrix& operator-=(const Matrix& M);
    
    bool operator==(const Matrix& M) const;
    bool operator!=(const Matrix& M) const;
    
    // Decomposed matrices
    Matrix translationMatrix() const;
    Matrix scaleMatrix() const;
    Matrix rotationMatrix() const;

    Vector3 left() const;
    Vector3 right() const;
    Vector3 up() const;
    Vector3 down() const;
    Vector3 forward() const;
    Vector3 backward() const;
    Quaternion rotation() const;
	Vector3 translation() const;
    
    void up( const Vector3& v);
    void forward( const Vector3& v);
    void right( const Vector3& v);

	Matrix& multiply(float f);
	Matrix& multiply(const Matrix& M);
	Matrix& add(const Matrix& M);
	Matrix& sub(const Matrix& M);
    Matrix& translation(float X, float Y, float Z );
	Matrix& translation(const Vector3& XYZ);
	Matrix& rotation(const Vector3& f, const Vector3& u, const Vector3& r);
    Matrix& rotationX(float Angle );
    Matrix& rotationY(float Angle );
    Matrix& rotationZ(float Angle );
    Matrix& rotationYawPitchRoll( float Yaw, float Pitch, float Roll );
    Matrix& rotationYawPitchRoll(const Vector3& Angles );
    Matrix& rotationAxis(const Vector3& Axis, float Angle);
    Matrix& scale(float ScaleX, float ScaleY, float ScaleZ );
    Matrix& scale(const Vector3& Scalings );
    Matrix& scale(float Scaling );
	Vector3 scale() const;
    Matrix& setIdentity();
    Matrix& transpose();
    Matrix& invert();
    Matrix& cameraLookAt(const Vector3& Target, const Vector3& Up, const Vector3& Position );
	Matrix& lookAt(const Vector3& Direction, const Vector3& Up);
	Matrix& lookAt(const Vector3& Direction);
    Matrix& perspective(float Fovy, float AspectRatio, float NearPlane, float FarPlane );
    Matrix& orthographic(float Width, float Height, float Near, float Far );
    Vector3 transformVec4x4( const Vector3& v) const;
    Vector3 transformVec3x3( const Vector3& v) const;
    Matrix& lastElementDivision();
    float determinant() const;
	std::string toString() const;

	static const Matrix identity;
	static const Matrix zero;
    //static Matrix& fromPosition(const Vector3& position);
    //static Matrix& fromRotation(const Quaternion& rotation);

    static Matrix FromRotationAxis(const Vector3& forward, const Vector3& up);
    static Matrix RotationAxis(const Vector3& axis, float angle);
};


#endif /* defined(__RealtimeRending__Matrix__) */
