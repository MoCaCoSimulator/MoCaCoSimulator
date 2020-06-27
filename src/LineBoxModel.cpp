//
//  LineBoxModel.cpp
//  CGXcode
//
//  Created by Philipp Lensing on 10.10.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "LineBoxModel.h"

LineBoxModel::LineBoxModel( float Width, float Height, float Depth )
{
	VB.begin();

	float x = Width * 0.5f;
	float y = Height * 0.5f;
	float z = Depth * 0.5f;

	Vector3 v1, v2, v3, v4, v5, v6, v7, v8;
	v1 = Vector3(x, y, z);
	v2 = Vector3(-x, y, z);
	v3 = Vector3(x, -y, z);
	v4 = Vector3(x, y, -z);
	v5 = -v4;
	v6 = -v3;
	v7 = -v2;
	v8 = -v1;
	
	VB.addVertex(v1);
	VB.addVertex(v2);
	VB.addVertex(v6);
	VB.addVertex(v4);

	VB.addVertex(v7);
	VB.addVertex(v8);
	VB.addVertex(v6);
	VB.addVertex(v8);

	VB.addVertex(v5);
	VB.addVertex(v2);
	VB.addVertex(v5);
	VB.addVertex(v3);

	VB.addVertex(v1);
	VB.addVertex(v3);
	VB.addVertex(v7);
	VB.addVertex(v4);

	VB.end();
}

void LineBoxModel::draw(const BaseCamera& Cam)
{
    BaseModel::draw(Cam);

	VB.activate();

	glDrawArrays(GL_LINE_LOOP, 0, VB.vertexCount());

	VB.deactivate();
}
