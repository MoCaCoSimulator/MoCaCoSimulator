//
//  Camera.cpp
//  RealtimeRending
//
//  Created by Philipp Lensing on 22.10.14.
//  Copyright (c) 2014 Philipp Lensing. All rights reserved.
//

#include "Camera.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "InputManager.h"

Camera::Camera(int width, int height) : 
	m_Position(0.0f,2.0f,3.0f),
	m_Target(0.0f,0.0f,0.0f),
	m_Up(0.0f,1.0f,0.0f),
    m_LastMouseX(-1), 
	m_LastMouseY(-1),
	m_Panning(0,0,0), 
	m_Zoom(0,0,0), 
	m_Rotation(0,0,0),
	m_NearPlane(0.045f),
	m_FarPlane(1000.0f),
	m_FOV(65.0f),
	WindowWidth(640), 
	WindowHeight(480),
	moving(false)
{
	WindowWidth = width;
	WindowHeight = height;

    setSize(WindowWidth, WindowHeight);
}

Camera::Camera() :
    m_Position(0.0f, 2.0f, 3.0f),
    m_Target(0.0f, 0.0f, 0.0f),
    m_Up(0.0f, 1.0f, 0.0f),
    m_LastMouseX(-1),
    m_LastMouseY(-1),
    m_Panning(0, 0, 0),
    m_Zoom(0, 0, 0),
    m_Rotation(0, 0, 0),
    m_NearPlane(0.045f),
    m_FarPlane(1000.0f),
    m_FOV(65.0f),
    WindowWidth(640),
    WindowHeight(480)
{
    setSize(WindowWidth, WindowHeight);
}

Vector3 Camera::position() const
{
    return m_Position + m_Panning + m_Zoom + m_Rotation;
}
Vector3 Camera::target() const
{
    return m_Target + m_Panning;
}
Vector3 Camera::up() const
{
    return m_Up;
}

void Camera::setPosition( const Vector3& Pos)
{
    m_Position = Pos;
    m_Panning = m_Rotation = m_Zoom = Vector3(0,0,0);
}

void Camera::setTarget( const Vector3& Target)
{
    m_Target = Target;
    m_Panning = Vector3(0,0,0);
}

void Camera::setUp( const Vector3& Up)
{
    m_Up = Up;
}

void Camera::setSize(int width, int height)
{
    WindowWidth = width;
    WindowHeight = height;
    m_ViewMatrix.setIdentity();
    m_ProjMatrix.perspective((float)M_PI * m_FOV / 180.0f, (float)WindowWidth / (float)WindowHeight, m_NearPlane, m_FarPlane);
    m_InvProjMatrix = Matrix(m_ProjMatrix).invert();
}

void Camera::handleInput()
{
	float x = InputManager::mousePosition.x;
	float y = InputManager::mousePosition.y;

	if (m_LastMouseX == -1)
		m_LastMouseX = x;
	if (m_LastMouseY == -1)
		m_LastMouseY = y;

	moving = InputManager::GetKey(InputManager::Keycode::Alt);

	if (moving)
	{
		if (InputManager::GetMouseButton(InputManager::Mousecode::Left))
			rotate((float)x * 1.f, (float)y * 1.f);
		else if (InputManager::GetMouseButton(InputManager::Mousecode::Middle))
			pan((float)(m_LastMouseX - x) * 0.01f, (float)(m_LastMouseY - y) * 0.01f);
		else if (InputManager::GetMouseButton(InputManager::Mousecode::Right))
			zoom((float)(m_LastMouseX - x) * -0.01f);
	}

	m_Position += m_Panning + m_Zoom + m_Rotation;
	m_Target += m_Panning;
	m_Panning = Vector3(0, 0, 0);
	m_Zoom = Vector3(0, 0, 0);
	m_Rotation = Vector3(0, 0, 0);

	m_LastMouseX = x;
	m_LastMouseY = y;
}

Vector3 Camera::screenPosToRay(float mouseX, float mouseY) const
{
	float x = 2.0f * mouseX / WindowWidth - 1.0f;
	float y = 1.0f - 2.0f * mouseY / WindowHeight;

	Vector4 rayEye = m_InvProjMatrix * Vector4(x, y, -1.0f, 1.0f);
	rayEye.z = -1.0f;
	rayEye.w = 0.0f;
	Vector4 rayWorld = Matrix(m_ViewMatrix).invert() * rayEye;

	Vector3 rayVec3 = Vector3(rayWorld.x, rayWorld.y, rayWorld.z);
	rayVec3.normalize();

	return rayVec3;
}

void Camera::pan( float dx, float dy)
{
    // calculate panning-plane
    
    Vector3 aDir = m_Target-m_Position;
    aDir.normalize();
    Vector3 aRight = aDir.cross(m_Up);
    aRight.normalize();
    Vector3 aUp = aDir.cross(aRight);
    m_Panning = aRight * dx + aUp * dy;
}

void Camera::zoom( float dz)
{
    Vector3 aDir = m_Target-m_Position;
    float Dist = aDir.length();
    aDir.normalize();
  
	if( Dist-dz <= 1.0f)
	{
		m_Zoom = aDir * (Dist-1.0f);
		return;
	}
    
	m_Zoom = aDir * dz;
}

void Camera::rotate( float x, float y )
{
    Vector3 po = getVSpherePos((float) m_LastMouseX, (float)m_LastMouseY);
    Vector3 pn = getVSpherePos(x, y);
    
    float cosangle = po.dot(pn);
    if(cosangle>1.0f) cosangle=1.0f;
    if(cosangle<-1.0f) cosangle=-1.0f;
    
    const float angle = acos(cosangle);
    Vector3 RotAxis = pn.cross(po);
    RotAxis.normalize();
    
    //Vector Diff = m_Position-m_Target;
    Vector3 Diff(0,0,(m_Position-m_Target).length());
    
    Vector3 RotDiff = rotateAxisAngle(Diff, RotAxis, angle);
    
    Vector3 cdir = m_Target-m_Position;
    cdir.normalize();
    Vector3 cup = m_Up;
    Vector3 cright = cdir.cross(cup);
    cright.normalize();
    cup = cright.cross(cdir);

    Vector3 RotDiffW;
    RotDiffW.x = cright.x * RotDiff.x + cup.x * RotDiff.y +  -cdir.x * RotDiff.z;
    RotDiffW.y = cright.y * RotDiff.x + cup.y * RotDiff.y +  -cdir.y * RotDiff.z;
    RotDiffW.z = cright.z * RotDiff.x + cup.z * RotDiff.y +  -cdir.z * RotDiff.z;
    m_Rotation = RotDiffW - (m_Position-m_Target);
}

Vector3 Camera::rotateAxisAngle( Vector3 v, Vector3 n, float a)
{
    float co = cos(a);
    float si = sin(a);
    
    Vector3 o;
    Vector3 mx( n.x*n.x*(1.0f-co)+co, n.x*n.y*(1.0f-co)-n.z*si,n.x*n.z*(1.0f-co)+n.y*si );
    Vector3 my( n.x*n.y*(1.0f-co)+n.z*si, n.y*n.y*(1.0f-co)+co, n.y*n.z*(1.0f-co)-n.x*si );
    Vector3 mz( n.x*n.z*(1.0f-co)-n.y*si, n.z*n.y*(1.0f-co)+n.x*si, n.z*n.z*(1.0f-co)+co);
    o.x = mx.dot(v);
    o.y = my.dot(v);
    o.z = mz.dot(v);

    return o;
}

const Matrix& Camera::getViewMatrix() const
{
    return m_ViewMatrix;
}

const Matrix& Camera::getProjectionMatrix() const
{
    return m_ProjMatrix;
}

Vector3 Camera::getVSpherePos( float x, float y)
{
    Vector3 p( 1.0f*x/(float)WindowWidth*2.0f - 1.0f, 1.0f*y/(float)WindowHeight*2.0f -1.0f, 0);
    p.y = -p.y;
    
    float sqrLen = p.lengthSquared();

    if( sqrLen <= 1.0f)
    {
        p.z = sqrt( 1- sqrLen);
    }
    else
        p.normalize();
    
    return p;
    
}

void Camera::update()
{
    Vector3 pos = this->position(); //m_Position + m_Panning + m_Zoom + m_Rotation;
    Vector3 target = this->target(); //m_Target + m_Panning;
    m_ViewMatrix.cameraLookAt(target, m_Up, pos);
}