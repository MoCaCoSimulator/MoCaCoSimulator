
#ifndef __RealtimeRending__Camera__
#define __RealtimeRending__Camera__

#include <iostream>
#include "vector.h"
#include "matrix.h"
#include "MouseInput.h"

class BaseCamera
{
public:
    virtual void update() = 0;
    virtual const Matrix& getViewMatrix() const  = 0;
    virtual const Matrix& getProjectionMatrix() const  = 0;
    virtual Vector3 position() const  = 0;
    virtual ~BaseCamera() {};
};

class SimpleCamera : public BaseCamera
{
public:
	virtual void update() {}
	virtual const Matrix& getViewMatrix() const { return View; }
	virtual const Matrix& getProjectionMatrix() const { return Proj;  }
	virtual Vector3 position() const { Matrix m = View; m.invert(); return m.translation(); }
	void setViewMatrix(const Matrix& m) { View = m; }
	void setProjectionMatrix(const Matrix& m) { Proj = m; }
	virtual ~SimpleCamera() {};
protected:
	Matrix View;
	Matrix Proj;
};

class Camera : public BaseCamera
{
public:
    Camera(int width, int height);
    Camera();
    virtual ~Camera() {};
    
    virtual Vector3 position() const;
    Vector3 target() const;
    Vector3 up() const;
    
    void setPosition(const Vector3& Pos);
    void setTarget(const Vector3& Target);
    void setUp(const Vector3& Up);
    void setSize(int width, int height);

    void handleInput();
	Vector3 screenPosToRay(float x, float y) const;
    
    virtual void update();
    virtual const Matrix& getViewMatrix() const;
    virtual const Matrix& getProjectionMatrix() const;

	float nearPlane() { return m_NearPlane; }
	float farPlane() { return m_FarPlane; }

	const bool IsMoving() const { return moving; }
protected:    
    void pan(float dx, float dy);
    void zoom(float dz);
    void rotate(float x, float y );
    Vector3 getVSpherePos(float x, float y);
    Vector3 rotateAxisAngle(Vector3 v, Vector3 n, float a);
    
    Matrix m_ViewMatrix;
    Matrix m_ProjMatrix;
	Matrix m_InvProjMatrix;
    Vector3 m_Position;
    Vector3 m_Target;
    Vector3 m_Up;
    Vector3 m_Panning;
    Vector3 m_Zoom;
    Vector3 m_Rotation;
    int m_LastMouseX;
    int m_LastMouseY;
    int WindowWidth;
    int WindowHeight;

	float m_NearPlane;
	float m_FarPlane;
	float m_FOV;
	bool moving;
};


#endif /* defined(__RealtimeRending__Camera__) */
