
#include "Matrix.h"
#include "Quaternion.h"
#include "math.h"
#include <assert.h>
#include <qdebug.h>

#define WEAK_EPSILON 1e-4f

const Matrix Matrix::identity = Matrix(1, 0, 0, 0,
									   0, 1, 0, 0,
									   0, 0, 1, 0,
									   0, 0, 0, 1);
const Matrix Matrix::zero = Matrix(0, 0, 0, 0,
									0, 0, 0, 0,
									0, 0, 0, 0,
									0, 0, 0, 0);

Matrix::Matrix()
{
	*this = Matrix::zero;
}

Matrix::Matrix( float _00, float _01, float _02, float _03,
                float _10, float _11, float _12, float _13,
                float _20, float _21, float _22, float _23,
                float _30, float _31, float _32, float _33 ) :
                m00(_00), m01(_01), m02(_02), m03(_03),
                m10(_10), m11(_11), m12(_12), m13(_13),
                m20(_20), m21(_21), m22(_22), m23(_23),
                m30(_30), m31(_31), m32(_32), m33(_33)
{
}

Matrix::Matrix(const Matrix& other)
{
	*this = other;
}

Matrix::operator float*()
{
    return m;
}
Matrix::operator const float* const()
{
    return m;
}

Matrix Matrix::operator*(const Matrix& M) const
{
    Matrix Out = *this;
    Out.multiply(M);
    return Out;
}
Matrix& Matrix::operator*=(const Matrix& M)
{
    multiply(M);
    return *this;
}

Vector3 Matrix::operator*(const Vector3& v) const
{
	return transformVec4x4(v);
}

Vector4 Matrix::operator*(const Vector4& v) const
{
	float X = m00 * v.x + m01 * v.y + m02 * v.z + m03 * v.w;
	float Y = m10 * v.x + m11 * v.y + m12 * v.z + m13 * v.w;
	float Z = m20 * v.x + m21 * v.y + m22 * v.z + m23 * v.w;
	float W = m30 * v.x + m31 * v.y + m32 * v.z + m33 * v.w;
	return Vector4(X, Y, Z, W);
}

Quaternion Matrix::operator*(const Quaternion& q) const
{
    return rotation() * q;
    /*
	Matrix out = *this;
	out.multiply(q.toRotationMatrix());
	return out;
    */
}

Matrix Matrix::operator*(float f) const
{
	Matrix out = *this;
	out.multiply(f);
	return out;
}

Matrix& Matrix::operator*=(float f)
{
	multiply(f);
	return *this;
}

Matrix Matrix::operator+(const Matrix& M) const
{
	Matrix Out = *this;
	Out.add(M);
	return Out;
}
Matrix& Matrix::operator+=(const Matrix& M)
{
	add(M);
	return *this;
}
Matrix Matrix::operator-(const Matrix& M) const
{
	Matrix Out = *this;
	Out.sub(M);
	return Out;
}
Matrix& Matrix::operator-=(const Matrix& M)
{
	sub(M);
	return *this;
}


bool Matrix::operator==(const Matrix& M) const
{
    const float Epsilon = WEAK_EPSILON;
    return fabs(m00-M.m00)<=Epsilon && fabs(m01-M.m01)<=Epsilon && fabs(m02-M.m02)<=Epsilon && fabs(m03-M.m03)<=Epsilon &&
    fabs(m10-M.m10)<=Epsilon && fabs(m11-M.m11)<=Epsilon && fabs(m12-M.m12)<=Epsilon && fabs(m13-M.m13)<=Epsilon &&
    fabs(m20-M.m20)<=Epsilon && fabs(m21-M.m21)<=Epsilon && fabs(m22-M.m22)<=Epsilon && fabs(m23-M.m23)<=Epsilon &&
    fabs(m30-M.m30)<=Epsilon && fabs(m31-M.m31)<=Epsilon && fabs(m32-M.m32)<=Epsilon && fabs(m33-M.m33)<=Epsilon;

    return false;
}

Vector3 Matrix::transformVec4x4( const Vector3& v) const
{
    float X = m00*v.x + m01*v.y + m02*v.z + m03;
    float Y = m10*v.x + m11*v.y + m12*v.z + m13;
    float Z = m20*v.x + m21*v.y + m22*v.z + m23;
    float W = m30*v.x + m31*v.y + m32*v.z + m33;
    return Vector3( X/W, Y/W, Z/W);

}
Vector3 Matrix::transformVec3x3( const Vector3& v) const
{
    float X = m00*v.x + m01*v.y + m02*v.z;
    float Y = m10*v.x + m11*v.y + m12*v.z;
    float Z = m20*v.x + m21*v.y + m22*v.z;
    return Vector3( X, Y, Z);
}

Matrix& Matrix::lastElementDivision()
{
    *this = *this * (1.0f / m33);
    return *this;
}


bool Matrix::operator!=(const Matrix& M) const
{
    return !(*this==M);
}

// https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
Matrix Matrix::translationMatrix() const
{
    return Matrix(1, 0, 0, m03,
                  0, 1, 0, m13,
                  0, 0, 1, m23,
                  0, 0, 0, 1);
}

// https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
Matrix Matrix::scaleMatrix() const
{
    Vector3 scaleVector = scale();
    return Matrix(scaleVector.x, 0,             0,             0,
                  0,             scaleVector.y, 0,             0,
                  0,             0,             scaleVector.z, 0,
                  0,             0,             0,             1);
}

// https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati/417813
Matrix Matrix::rotationMatrix() const
{
    Vector3 scaleVector = scale();
    return Matrix(m00 / scaleVector.x, m01 / scaleVector.y, m02 / scaleVector.z, 0,
                  m10 / scaleVector.x, m11 / scaleVector.y, m12 / scaleVector.z, 0,
                  m20 / scaleVector.x, m21 / scaleVector.y, m22 / scaleVector.z, 0,
                  0                  , 0                  , 0                  , 1);
}

Vector3 Matrix::right() const
{
    return Vector3(m00, m10, m20);
}

Vector3 Matrix::left() const
{
	return Vector3(-m00, -m10, -m20);
}

Vector3 Matrix::up() const
{
    return Vector3(m01, m11, m21);
}

Vector3 Matrix::down() const
{
    return Vector3(-m01, -m11, -m21);
}

Vector3 Matrix::forward() const
{
    return Vector3(m02, m12, m22);
}

Vector3 Matrix::backward() const
{
    return Vector3(-m02, -m12, -m22);
}

Vector3 Matrix::translation() const
{
    return Vector3(m03, m13, m23);
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
Quaternion Matrix::rotation() const
{
	Quaternion q;
	float absQ2 = std::pow((double)determinant(), 1.0 / 3.0);
	q.x = std::sqrt(std::max(0.0f, absQ2 + m00 - m11 - m22)) / 2;
	q.y = std::sqrt(std::max(0.0f, absQ2 - m00 + m11 - m22)) / 2;
	q.z = std::sqrt(std::max(0.0f, absQ2 - m00 - m11 + m22)) / 2;
	q.w = std::sqrt(std::max(0.0f, absQ2 + m00 + m11 + m22)) / 2;
	q.x *= sgn(q.x * (m21 - m12));
	q.y *= sgn(q.y * (m02 - m20));
	q.z *= sgn(q.z * (m10 - m01));
	return q.normalized();
}

void Matrix::up( const Vector3& v)
{
    m01 = v.x;
    m11 = v.y;
    m21 = v.z;
}

void Matrix::forward( const Vector3& v)
{
    m02 = v.x;
    m12 = v.y;
    m22 = v.z;
}

void Matrix::right( const Vector3& v)
{
    m00 = v.x;
    m10 = v.y;
    m20 = v.z;
}

Matrix& Matrix::multiply(const Matrix& M)
{
	const Matrix& A = *this;

	Matrix Tmp(
		(A.m00 * M.m00 + A.m01 * M.m10 + A.m02 * M.m20 + A.m03 * M.m30),
		(A.m00 * M.m01 + A.m01 * M.m11 + A.m02 * M.m21 + A.m03 * M.m31),
		(A.m00 * M.m02 + A.m01 * M.m12 + A.m02 * M.m22 + A.m03 * M.m32),
		(A.m00 * M.m03 + A.m01 * M.m13 + A.m02 * M.m23 + A.m03 * M.m33),

		(A.m10 * M.m00 + A.m11 * M.m10 + A.m12 * M.m20 + A.m13 * M.m30),
		(A.m10 * M.m01 + A.m11 * M.m11 + A.m12 * M.m21 + A.m13 * M.m31),
		(A.m10 * M.m02 + A.m11 * M.m12 + A.m12 * M.m22 + A.m13 * M.m32),
		(A.m10 * M.m03 + A.m11 * M.m13 + A.m12 * M.m23 + A.m13 * M.m33),

		(A.m20 * M.m00 + A.m21 * M.m10 + A.m22 * M.m20 + A.m23 * M.m30),
		(A.m20 * M.m01 + A.m21 * M.m11 + A.m22 * M.m21 + A.m23 * M.m31),
		(A.m20 * M.m02 + A.m21 * M.m12 + A.m22 * M.m22 + A.m23 * M.m32),
		(A.m20 * M.m03 + A.m21 * M.m13 + A.m22 * M.m23 + A.m23 * M.m33),

		(A.m30 * M.m00 + A.m31 * M.m10 + A.m32 * M.m20 + A.m33 * M.m30),
		(A.m30 * M.m01 + A.m31 * M.m11 + A.m32 * M.m21 + A.m33 * M.m31),
		(A.m30 * M.m02 + A.m31 * M.m12 + A.m32 * M.m22 + A.m33 * M.m32),
		(A.m30 * M.m03 + A.m31 * M.m13 + A.m32 * M.m23 + A.m33 * M.m33));
	*this = Tmp;
	return *this;
}

Matrix& Matrix::multiply(float f)
{
	m00 *= f;	m01 *= f;	m02 *= f;	m03 *= f;
	m10 *= f;	m11 *= f;	m12 *= f;	m13 *= f;
	m20 *= f;	m21 *= f;	m22 *= f;	m23 *= f;
	m30 *= f;	m31 *= f;	m32 *= f;	m33 *= f;
	return *this;
}
Matrix& Matrix::add(const Matrix& M)
{
	m00 += M.m00;	m01 += M.m01;	m02 += M.m02;	m03 += M.m03;
	m10 += M.m10;	m11 += M.m11;	m12 += M.m12;	m13 += M.m13;
	m20 += M.m20;	m21 += M.m21;	m22 += M.m22;	m23 += M.m23;
	m30 += M.m30;	m31 += M.m31;	m32 += M.m32;	m33 += M.m33;
	return *this;
}
Matrix& Matrix::sub(const Matrix& M)
{
	m00 -= M.m00;	m01 -= M.m01;	m02 -= M.m02;	m03 -= M.m03;
	m10 -= M.m10;	m11 -= M.m11;	m12 -= M.m12;	m13 -= M.m13;
	m20 -= M.m20;	m21 -= M.m21;	m22 -= M.m22;	m23 -= M.m23;
	m30 -= M.m30;	m31 -= M.m31;	m32 -= M.m32;	m33 -= M.m33;
	return *this;
}
Matrix& Matrix::translation(float X, float Y, float Z )
{
    m00= 1;	m01= 0;	m02= 0;	m03= X;
    m10= 0;	m11= 1;	m12= 0;	m13= Y;
    m20= 0;	m21= 0;	m22= 1;	m23= Z;
    m30= 0;	m31= 0;	m32= 0;	m33= 1;
    return *this;
}

Matrix& Matrix::translation(const Vector3& XYZ )
{
    return translation(XYZ.x, XYZ.y, XYZ.z);
}

Matrix& Matrix::rotation(const Vector3& f, const Vector3& u, const Vector3& r)
{
	m00 = r.x;  m01 = u.x;  m02 = f.x;   m03 = 0;
	m10 = r.y;  m11 = u.y;  m12 = f.y;   m13 = 0;
	m20 = r.z;  m21 = u.z;  m22 = f.z;  m23 = 0;
	m30 = 0;	m31 = 0;	m32 = 0;	m33 = 1;
	return *this;
}

Matrix& Matrix::rotationX(float Angle )
{
    m00= 1;	m01= 0;	m02= 0;	m03= 0;
    m10= 0;					m13= 0;
    m20= 0;					m23= 0;
    m30= 0;	m31= 0;	m32= 0;	m33= 1;

    m11 = m22 = cos(Angle);
    m21 = sin(Angle);
    m12 = -m21;

    return *this;
}
Matrix& Matrix::rotationY(float Angle )
{
            m01= 0;         m03= 0;
    m10= 0;	m11= 1;	m12= 0;	m13= 0;
            m21= 0;         m23= 0;
    m30= 0;	m31= 0;	m32= 0;	m33= 1;

    m00 = m22 = cos(Angle);
    m02 = sin(Angle);
    m20 = -m02;

    return *this;
}
Matrix& Matrix::rotationZ(float Angle )
{
                    m02= 0;	m03= 0;
                    m12= 0;	m13= 0;
    m20= 0;	m21= 0;	m22= 1;	m23= 0;
    m30= 0;	m31= 0;	m32= 0;	m33= 1;

    m00 = m11 = cos(Angle);
    m10= sin(Angle);
    m01= -m10;

    return *this;
}
Matrix& Matrix::rotationYawPitchRoll( float Yaw, float Pitch, float Roll )
{
    float cosx = cos(Pitch);
    float cosy = cos(Yaw);
    float cosz = cos(Roll);

    float sinx = sin(Pitch);
    float siny = sin(Yaw);
    float sinz = sin(Roll);

    m00 = cosz*cosy + sinz*sinx*siny;
    m10 = sinz*cosx;
    m20 = -cosz*siny + sinz*sinx*cosy;
    m30 = 0;

    m01 = -sinz*cosy + cosz*sinx*siny;
    m11 = cosz*cosx;
    m21 = sinz*siny + cosz*sinx*cosy;
    m31 = 0;

    m02 = cosx*siny;
    m12 = -sinx;
    m22 = cosx*cosy;
    m32 = 0;

    m03 = m13 = m23 = 0;
    m33 = 1;

    return *this;
}
Matrix& Matrix::rotationYawPitchRoll(const Vector3& Angles )
{
    rotationYawPitchRoll(Angles.x, Angles.y, Angles.z);
    return *this;
}

Matrix& Matrix::rotationAxis(const Vector3& Axis, float Angle)
{
    const float Si = sin(Angle);
    const float Co = cos(Angle);
    const float OMCo = 1 - Co;
    Vector3 Ax = Axis;
    Ax.normalize();

    m00= (Ax.x * Ax.x) * OMCo + Co;
    m01= (Ax.x * Ax.y) * OMCo - (Ax.z * Si);
    m02= (Ax.x * Ax.z) * OMCo + (Ax.y * Si);
    m03= 0;

    m10= (Ax.y * Ax.x) * OMCo + (Ax.z * Si);
    m11= (Ax.y * Ax.y) * OMCo + Co;
    m12= (Ax.y * Ax.z) * OMCo - (Ax.x * Si);
    m13= 0;

    m20= (Ax.z * Ax.x) * OMCo - (Ax.y * Si);
    m21= (Ax.z * Ax.y) * OMCo + (Ax.x * Si);
    m22= (Ax.z * Ax.z) * OMCo + Co;
    m23= 0;

    m30= 0;
    m31= 0;
    m32= 0;
    m33= 1;

    return *this;
}
Matrix& Matrix::scale(float ScaleX, float ScaleY, float ScaleZ )
{
    m00= ScaleX;	m01= 0;			m02= 0;			m03= 0;
    m10= 0;			m11= ScaleY;	m12= 0;			m13= 0;
    m20= 0;			m21= 0;			m22= ScaleZ;	m23= 0;
    m30= 0;			m31= 0;			m32= 0;			m33= 1;

    return *this;
}
Matrix& Matrix::scale(const Vector3& Scalings )
{
    scale( Scalings.x, Scalings.y, Scalings.z);
    return *this;
}
Matrix& Matrix::scale(float Scaling )
{
    scale(Scaling, Scaling, Scaling);
    return *this;
}
Vector3 Matrix::scale() const
{
	Vector3 scale;
	scale.x = sqrt(m00 * m00 + m10 * m10 + m20 * m20 + m30 * m30);
	scale.y = sqrt(m01 * m01 + m11 * m11 + m21 * m21 + m31 * m31);
	scale.z = sqrt(m02 * m02 + m12 * m12 + m22 * m22 + m32 * m32);
	return scale;
}
Matrix& Matrix::setIdentity()
{
    m00= 1;	m01= 0;	m02= 0;	m03= 0;
    m10= 0;	m11= 1;	m12= 0;	m13= 0;
    m20= 0;	m21= 0;	m22= 1;	m23= 0;
    m30= 0;	m31= 0;	m32= 0;	m33= 1;
    return *this;
}
Matrix& Matrix::transpose()
{
    Matrix Tmp(
      m00, m10, m20, m30,
      m01, m11, m21, m31,
      m02, m12, m22, m32,
      m03, m13, m23, m33 );
    *this = Tmp;
    return *this;
}
Matrix& Matrix::invert()
{
    const float num5 = m00;
    const float num4 = m01;
    const float num3 = m02;
    const float num2 = m03;
    const float num9 = m10;
    const float num8 = m11;
    const float num7 = m12;
    const float num6 = m13;
    const float num17 = m20;
    const float num16 = m21;
    const float num15 = m22;
    const float num14 = m23;
    const float num13 = m30;
    const float num12 = m31;
    const float num11 = m32;
    const float num10 = m33;
    const float num23 = (num15 * num10) - (num14 * num11);
    const float num22 = (num16 * num10) - (num14 * num12);
    const float num21 = (num16 * num11) - (num15 * num12);
    const float num20 = (num17 * num10) - (num14 * num13);
    const float num19 = (num17 * num11) - (num15 * num13);
    const float num18 = (num17 * num12) - (num16 * num13);
    const float num39 = ((num8 * num23) - (num7 * num22)) + (num6 * num21);
    const float num38 = -(((num9 * num23) - (num7 * num20)) + (num6 * num19));
    const float num37 = ((num9 * num22) - (num8 * num20)) + (num6 * num18);
    const float num36 = -(((num9 * num21) - (num8 * num19)) + (num7 * num18));
    const float num = (float)1 / ((((num5 * num39) + (num4 * num38)) + (num3 * num37)) + (num2 * num36));
    m00 = num39 * num;
    m10 = num38 * num;
    m20 = num37 * num;
    m30 = num36 * num;
    m01 = -(((num4 * num23) - (num3 * num22)) + (num2 * num21)) * num;
    m11 = (((num5 * num23) - (num3 * num20)) + (num2 * num19)) * num;
    m21 = -(((num5 * num22) - (num4 * num20)) + (num2 * num18)) * num;
    m31 = (((num5 * num21) - (num4 * num19)) + (num3 * num18)) * num;
    const float num35 = (num7 * num10) - (num6 * num11);
    const float num34 = (num8 * num10) - (num6 * num12);
    const float num33 = (num8 * num11) - (num7 * num12);
    const float num32 = (num9 * num10) - (num6 * num13);
    const float num31 = (num9 * num11) - (num7 * num13);
    const float num30 = (num9 * num12) - (num8 * num13);
    m02 = (((num4 * num35) - (num3 * num34)) + (num2 * num33)) * num;
    m12 = -(((num5 * num35) - (num3 * num32)) + (num2 * num31)) * num;
    m22 = (((num5 * num34) - (num4 * num32)) + (num2 * num30)) * num;
    m32 = -(((num5 * num33) - (num4 * num31)) + (num3 * num30)) * num;
    const float num29 = (num7 * num14) - (num6 * num15);
    const float num28 = (num8 * num14) - (num6 * num16);
    const float num27 = (num8 * num15) - (num7 * num16);
    const float num26 = (num9 * num14) - (num6 * num17);
    const float num25 = (num9 * num15) - (num7 * num17);
    const float num24 = (num9 * num16) - (num8 * num17);
    m03 = -(((num4 * num29) - (num3 * num28)) + (num2 * num27)) * num;
    m13 = (((num5 * num29) - (num3 * num26)) + (num2 * num25)) * num;
    m23 = -(((num5 * num28) - (num4 * num26)) + (num2 * num24)) * num;
    m33 = (((num5 * num27) - (num4 * num25)) + (num3 * num24)) * num;

    return *this;
}
Matrix& Matrix::cameraLookAt(const Vector3& Target, const Vector3& Up, const Vector3& Position)
{
	Vector3 f = Target - Position;
	f.normalize();
	Vector3 u = Up;
	u.normalize();
	Vector3 r = f.cross(u);
	r.normalize();
	u = r.cross(f);
	m00 = r.x;   m01 = r.y;   m02 = r.z;   m03 = -(r.dot(Position));
	m10 = u.x;   m11 = u.y;   m12 = u.z;   m13 = -(u.dot(Position));
	m20 = -f.x;  m21 = -f.y;  m22 = -f.z;  m23 = (f.dot(Position));
	m30 = 0;     m31 = 0;     m32 = 0;     m33 = 1;

	return *this;
}

Matrix& Matrix::lookAt(const Vector3& Direction, const Vector3& Up)
{
	Vector3 f = Direction;
	f.normalize();
	Vector3 u = Up;
	u.normalize();

	//check if forward and up are parallel
	if (abs(f.dot(u)) > 0.9995f)
	{
		//check if forward and global up are parallel
		if (abs(f.dot(Vector3::up)) > 0.9995f)
			u = f.cross(Vector3::right).normalized();
		else
			u = Vector3::up;
	}

	//compute right from forward and up
	Vector3 r = u.cross(f);
	r.normalize();

	//compute correct up
	u = f.cross(r);
	u.normalize();

	rotation(f, u, r);
	return *this;
}

Matrix& Matrix::lookAt(const Vector3& Direction)
{
	return lookAt(Direction, Vector3::up);
}

Matrix& Matrix::perspective(float Fovy, float AspectRatio, float NearPlane, float FarPlane )
{
    assert(NearPlane<FarPlane);

    const float f = 1.0f/tan(Fovy*0.5f);
    const float NearMinusFar = NearPlane-FarPlane;

    m01 = m02 = m03 = 0;
    m10 = m12 = m13 = 0;
    m20 = m21 = 0;
    m30 = m31 = m33 = 0;
    m32 = -1;

    m00 = f / AspectRatio;
    m11 = f;
    m22 = (FarPlane+NearPlane)/NearMinusFar;
    m23 = 2.0f*FarPlane*NearPlane/NearMinusFar;
    return *this;
}
Matrix& Matrix::orthographic(float Width, float Height, float Near, float Far )
{
    float FMN = 1.0f/(Far-Near);
    m00 = 2.0f/Width;   m01 = 0.0f;         m02 = 0.0f;      m03 = 0.0f;
    m10 = 0.0f;         m11 = 2.0f/Height;  m12 = 0.0f;      m13 = 0.0f;
    m20 = 0.0f;         m21 = 0.0f;         m22 = -2.0f*FMN; m23 = -(Far+Near)*FMN;
    m30 = 0.0f;         m31 = 0.0f;         m32 = 0.0f;      m33 = 1.0f;
    return *this;
}
float Matrix::determinant() const
{
    return	m00 * (m11 * m22 - m12 * m21) -
    m01 * (m10 * m22 - m12 * m20) +
    m02 * (m10 * m21 - m11 * m20);
}
std::string Matrix::toString() const
{
	std::ostringstream oss;
	oss << m00 << " " << m01 << " " << m02 << " " << m03 << "\n " << m10 << " " << m11 << " " << m12 << " " << m13 << "\n " << m20 << " " << m21 << " " << m22 << " " << m23 << "\n " << m30 << " " << m31 << " " << m32 << " " << m33 << "\n";
	return oss.str();
}
Matrix Matrix::FromRotationAxis(const Vector3& forward, const Vector3& up)
{
	return Matrix().lookAt(forward, up);
}

Matrix Matrix::RotationAxis(const Vector3& axis, float angle)
{
    return Matrix().rotationAxis(axis, angle);
}
