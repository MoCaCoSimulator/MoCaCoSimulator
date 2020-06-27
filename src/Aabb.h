//
//  Aabb.hpp
//  CGXcode
//
//  Created by Philipp Lensing on 02.11.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef Aabb_hpp
#define Aabb_hpp

#include <stdio.h>
#include "vector.h"
#include "matrix.h"

class AABB
{
public:
    Vector3 Min;
    Vector3 Max;
    AABB();
    AABB(const Vector3& min, const Vector3& max);
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
    Vector3 size() const;
	AABB transform(const Matrix& m) const;
	AABB merge(const AABB& a, const AABB& b) const;
	AABB& merge(const AABB& a);
	Vector3 center() const;
	void corners(Vector3 c[8]) const;
	void fromPoints(const Vector3* Points, unsigned int PointCount);
    static const AABB& unitBox();
protected:
    static AABB UnitBox;
};



#endif /* Aabb_hpp */
