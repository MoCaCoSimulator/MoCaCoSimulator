#pragma once

#ifndef hit_h
#define hit_h

#include "vector.h"
#include "BaseModel.h"
#include <qdebug.h>

class BaseModel;

class HitInfo
{
public:
	struct TriangleInfo {
	public:
		int indexA, indexB, indexC;
		Vector3 posA, posB, posC;

		TriangleInfo()
			: indexA(-1)
			, indexB(-1)
			, indexC(-1)
			, posA(Vector3::zero)
			, posB(Vector3::zero)
			, posC(Vector3::zero)
			, cachedNormal(NULL)
		{
		}

		~TriangleInfo()
		{
			if (cachedNormal)
				delete cachedNormal;
		}

		const Vector3 normal()
		{
			if (!cachedNormal)
			{
				Vector3 n = ((posB - posA).cross(posC - posA)).normalize();
				if (n.lengthSquared() == 0.0f)
					n = Vector3::up;
				cachedNormal = new Vector3(n);
			}
			return *cachedNormal;
		}

		TriangleInfo& operator=(const TriangleInfo& other) 
		{
			indexA = other.indexA;
			indexB = other.indexB;
			indexC = other.indexC;
			posA = other.posA;
			posB = other.posB;
			posC = other.posC;
			cachedNormal = other.cachedNormal;

			return (*this);
		}
	private: 
		Vector3* cachedNormal;
	};

	HitInfo() : position(Vector3::zero), distance(FLT_MAX), model(NULL), meshID(-1), hit(false) {};
	HitInfo& operator=(const HitInfo& other);

	Vector3 position;
	float distance;
	BaseModel* model;
	int meshID;
	bool hit;
	TriangleInfo triangleInfo;
};

#endif