#ifndef LIGHTS_H
#define LIGHTS_H

#include "vector.h"
#include "color.h"

class BaseLight
{
public:
	enum LightType
	{
		POINT = 0,
		DIRECTIONAL,
		SPOT
	};

	BaseLight(const Vector3& p = Vector3(10, 10, 10), const Color& c = ::Color(1, 1, 1)) : Position(p), Col(c), Attenuation(1,0,0), ShadowCaster(false) {}
	virtual ~BaseLight() {}

	const Vector3& position() const { return Position; }
	void position(const Vector3& p) { Position = p; }

	const Vector3& attenuation() const { return Attenuation; }
	void attenuation(const Vector3& a) { Attenuation = a; }

	const Color& color() const { return Col; }
	void color(const Color& c) { Col = c; }

	bool castShadows() const { return type() == POINT ? false : ShadowCaster;  }
	void castShadows(bool b) { type()==POINT ? ShadowCaster = false : ShadowCaster = b;  }

	virtual LightType type() const = 0;

protected:
	Vector3 Position;
	Vector3 Attenuation;
	Color Col;
	bool ShadowCaster;
};

class PointLight : public BaseLight
{
public:
	PointLight(const Vector3& p = Vector3(10, 10, 10), const Color& c = ::Color(1, 1, 1)) : BaseLight(p,c) {}
	virtual ~PointLight() {}

	virtual LightType type() const { return POINT; }
};

class DirectionalLight : public BaseLight
{
public:
	DirectionalLight(const Vector3& d = Vector3(-1, -1, -1), const Color& c = ::Color(1, 1, 1)) : BaseLight(Vector3(10,10,10), c), Direction(d) {}
	virtual ~DirectionalLight() {}

	virtual LightType type() const { return DIRECTIONAL; }

	const Vector3& direction() const { return Direction; }
	void direction(const Vector3& d) { Direction = d; }

protected:
	Vector3 Direction;
};

class SpotLight : public BaseLight
{
public:
	SpotLight(const Vector3& p = Vector3(10, 10, 10), const Vector3& d = Vector3(-1, -1, -1), float InnerRadius = 30.0, float OuterRadius = 40.0f, const Color& c = ::Color(1, 1, 1)) : BaseLight(p, c), Direction(d), InnerRadius(InnerRadius), OuterRadius(OuterRadius) {}
	virtual ~SpotLight() {}

	virtual LightType type() const { return SPOT; }

	float innerRadius() const { return InnerRadius; }
	void innerRadius(float r) { InnerRadius = r; }

	float outerRadius() const { return OuterRadius; }
	void outerRadius(float r) { OuterRadius = r; }

	const Vector3& direction() const { return Direction; }
	void direction(const Vector3& d) { Direction = d; }

protected:
	float InnerRadius;
	float OuterRadius;
	Vector3 Direction;
};



#endif
