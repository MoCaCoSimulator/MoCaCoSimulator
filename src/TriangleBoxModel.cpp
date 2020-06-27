//
//  TriangleBoxModel.cpp
//  CGXcode
//
//  Created by Philipp Lensing on 10.10.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "TriangleBoxModel.h"

TriangleBoxModel::TriangleBoxModel(float Width, float Height, float Depth)
{
	float x = Width * 0.5f;
	float y = Height * 0.5f;
	float z = Depth * 0.5f;

	VB.begin();
	
	//First plane
	Vector3 n;
	for (int j = 1; j <= 6; ++j)
	{
		float c = (float)(j % 2) == 0 ? -1.f: 1.f;
		for (int i = 1; i <= 4; ++i)
		{
			float s = (float)(i % 2) == 0 ? 1.f : 0.f;
			float t = i > 2 ? 1.f : 0.f;
			VB.addTexcoord0( s, t );

			//Changing s & t 0 to -1
			s = s > 0 ? s : -1.f;
			t = t > 0 ? t : -1.f;

			Vector3 mult;
			if (j > 4) mult = Vector3(s, t, -c);
			else if (j > 2) mult = Vector3(c, t, s);
			else mult = Vector3(-s, c, -t);

			Vector3 vec;
			vec.x = mult.x * x;
			vec.y = mult.y * y;
			vec.z = mult.z * z;
			VB.addNormal(Vector3(vec).normalize());
			VB.addVertex(vec);
		}
	}

	VB.end();

	IB.begin();
	
	for (int i = 0; i < 6; ++i)
	{
		int it = i * 4;
		if (i % 2 == 0 && i != 5)
		{
			IB.addIndex(it + 0); IB.addIndex(it + 2); IB.addIndex(it + 3);
			IB.addIndex(it + 3); IB.addIndex(it + 1); IB.addIndex(it + 0);
		}
		else
		{
			IB.addIndex(it + 3); IB.addIndex(it + 2); IB.addIndex(it + 0);
			IB.addIndex(it + 0); IB.addIndex(it + 1); IB.addIndex(it + 3);
		}
	}

	IB.end();
}

void TriangleBoxModel::draw(const BaseCamera& Cam)
{
    BaseModel::draw(Cam);

	VB.activate();
	IB.activate();

	glDrawElements(GL_TRIANGLES, IB.indexCount(), IB.indexFormat(), 0);

	IB.deactivate();
	VB.deactivate();
    
	// TODO: Add your code (Exercise 2)
}
