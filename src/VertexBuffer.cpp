//
//  VertexBuffer.cpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "VertexBuffer.h"
#include <assert.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

VertexBuffer::VertexBuffer() : ActiveAttributes(0), WithinBeginBlock(false), VAO(0), VBO(0), VertexCount(0), BuffersInitialized(false)
{
    
}

VertexBuffer::~VertexBuffer()
{
    if(BuffersInitialized)
    {
        glDeleteVertexArrays(1,&VAO);
        glDeleteBuffers(1, &VBO);
    }
}

void VertexBuffer::begin()
{
    if(BuffersInitialized)
    {
        glDeleteVertexArrays(1,&VAO);
        glDeleteBuffers(1, &VBO);
    }
    BuffersInitialized = false;
    VertexCount = 0;
    
    ActiveAttributes = 0;
    Vertices.clear();
    Normals.clear();
    Colors.clear();
    Texcoord0.clear();
    Texcoord1.clear();
    Texcoord2.clear();
    Texcoord3.clear();
	Weights.clear();
    WithinBeginBlock = true;
}

void VertexBuffer::addNormal( float x, float y, float z)
{
    if(!WithinBeginBlock) { std::cout << "call addNormal only between begin and end method!\n"; return; }
    ActiveAttributes |= NORMAL;
    Normals.push_back( Vector3(x,y,z));
}

void VertexBuffer::addNormal( const Vector3& v)
{
    if(!WithinBeginBlock) { std::cout << "call addNormal only between begin and end method!\n"; return; }
    ActiveAttributes |= NORMAL;
    Normals.push_back( v);
}

void VertexBuffer::addColor( const Color& c)
{
    if(!WithinBeginBlock) { std::cout << "call addColor only between begin and end method!\n"; return; }
    ActiveAttributes |= COLOR;
    Colors.push_back( c);
}

void VertexBuffer::addTexcoord0( float s, float t, float u )
{
    if(!WithinBeginBlock) { std::cout << "call addTexcoord0 only between begin and end method!\n"; return; }
    ActiveAttributes |= TEXCOORD0;
    Texcoord0.push_back(Vector3(s, t, u));
}

void VertexBuffer::addTexcoord1( float s, float t, float u )
{
    if(!WithinBeginBlock) { std::cout << "call addTexcoord1 only between begin and end method!\n"; return; }
    ActiveAttributes |= TEXCOORD1;
    Texcoord1.push_back(Vector3(s, t, u));
}
void VertexBuffer::addTexcoord2( float s, float t, float u )
{
    if(!WithinBeginBlock) { std::cout << "call addTexcoord2 only between begin and end method!\n"; return; }
    ActiveAttributes |= TEXCOORD2;
    Texcoord2.push_back(Vector3(s, t, u));
}
void VertexBuffer::addTexcoord3( float s, float t, float u )
{
    if(!WithinBeginBlock) { std::cout << "call addTexcoord3 only between begin and end method!\n"; return; }
    ActiveAttributes |= TEXCOORD3;
    Texcoord3.push_back(Vector3(s, t, u));
}

void VertexBuffer::addJointWeights(const JointWeights& weights)
{
	if (!WithinBeginBlock) { std::cout << "call addJointWeights only between begin and end method!\n"; return; }
	ActiveAttributes |= JOINTS;
	Weights.push_back(weights.Sorted());
}

void VertexBuffer::addVertex( float x, float y, float z)
{
    addVertex( Vector3(x,y,z) );
}

void VertexBuffer::addVertex( const Vector3& v)
{
    if(!WithinBeginBlock) { std::cout << "call addVertex only between begin and end method!\n"; return; }
    ActiveAttributes |= VERTEX;
    
    Vertices.push_back( v);

    if( ActiveAttributes&NORMAL)
        while( Normals.size() < Vertices.size() )
            Normals.push_back(Normals.back()); // copy last element
    if( ActiveAttributes&COLOR)
        while( Colors.size() < Vertices.size() )
            Colors.push_back(Colors.back()); // copy last element
    if( ActiveAttributes&TEXCOORD0)
        while( Texcoord0.size() < Vertices.size() )
            Texcoord0.push_back(Texcoord0.back()); // copy last element
    if( ActiveAttributes&TEXCOORD1)
        while( Texcoord1.size() < Vertices.size() )
            Texcoord1.push_back(Texcoord1.back()); // copy last element
    if( ActiveAttributes&TEXCOORD2)
        while( Texcoord2.size() < Vertices.size() )
            Texcoord2.push_back(Texcoord2.back()); // copy last element
	if (ActiveAttributes & TEXCOORD3)
		while (Texcoord3.size() < Vertices.size())
			Texcoord3.push_back(Texcoord3.back()); // copy last element
	if (ActiveAttributes & JOINTS)
		while (Weights.size() < Vertices.size())
			Weights.push_back(Weights.back()); // copy last element
 
    VertexCount = (unsigned int) Vertices.size();
}

void VertexBuffer::end()
{
    WithinBeginBlock = false;

    VertexCount = (unsigned int) Vertices.size();
    if(VertexCount == 0)
    {
        std::cout << "VertexBuffer::end(): no vertices found. call addVertex() within begin() and end() method\n";
        return;
    }

    if( ((ActiveAttributes&NORMAL) && (VertexCount != (unsigned int)Normals.size())) ||
        ((ActiveAttributes&COLOR) && (VertexCount != (unsigned int)Colors.size())) ||
       ((ActiveAttributes&TEXCOORD0) && (VertexCount != (unsigned int)Texcoord0.size())) ||
       ((ActiveAttributes&TEXCOORD1) && (VertexCount != (unsigned int)Texcoord1.size())) )
    {
        std::cout << "VertexBuffer::end(): vertex count and attribute(s) count do not match! Please ensure that #vertices=#normals, #vertices=#colors, #vertices=#texcoord\n";
		std::cout << "#vertices = " << VertexCount << " #normals = " << (ActiveAttributes & NORMAL) << " #colors = " << (ActiveAttributes & COLOR) << " #texcoord0&1 = " << (ActiveAttributes & TEXCOORD0) << " " << (ActiveAttributes & TEXCOORD1) << std::endl;
		std::cout << "#vertices = " << VertexCount << " #normals = " << Normals.size() << " #colors = " << Colors.size() << " #texcoord0&1 = " << Texcoord0.size() << " " << Texcoord1.size() << std::endl;
		return;
    }

	/*GLuint ElementSize = 4 * sizeof(float) +
		((ActiveAttributes & NORMAL) ? 4 * sizeof(float) : 0) +
		((ActiveAttributes & COLOR) ? 4 * sizeof(float) : 0) +
		((ActiveAttributes & TEXCOORD0) ? 3 * sizeof(float) : 0) +
		((ActiveAttributes & TEXCOORD1) ? 3 * sizeof(float) : 0) +
		((ActiveAttributes & TEXCOORD2) ? 3 * sizeof(float) : 0) +
		((ActiveAttributes & TEXCOORD3) ? 3 * sizeof(float) : 0) +
		((ActiveAttributes & JOINTS) ? JOINTCOUNT * (sizeof(float) + sizeof(float)) : 0);*/
	GLuint ElementSize = 4 * sizeof(float) +
		4 * sizeof(float) +
		((ActiveAttributes & COLOR) ? 4 * sizeof(float) : 0) +
		3 * sizeof(float) +
		3 * sizeof(float) +
		3 * sizeof(float) +
		3 * sizeof(float) +
		((ActiveAttributes & JOINTS) ? JOINTCOUNT * (sizeof(float) + sizeof(float)) : 0);
	GLuint BufferSize = (GLuint)Vertices.size() * ElementSize;

	char* ByteBuf = new char[BufferSize];

	assert(ByteBuf);
	float* Buffer = (float*)ByteBuf;

	--Buffer;
	for (unsigned long i = 0; i<VertexCount; ++i)
	{
		*(++Buffer) = Vertices[i].x;
		*(++Buffer) = Vertices[i].y;
		*(++Buffer) = Vertices[i].z;
		*(++Buffer) = 1.0f;

		if (ActiveAttributes&NORMAL)
		{
			*(++Buffer) = Normals[i].x;
			*(++Buffer) = Normals[i].y;
			*(++Buffer) = Normals[i].z;
			*(++Buffer) = 0.0f;
		}
		else
		{
			*(++Buffer) = 0.0f;
			*(++Buffer) = 1.0f;
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
		}

		if (ActiveAttributes&COLOR)
		{
			*(++Buffer) = Colors[i].R;
			*(++Buffer) = Colors[i].G;
			*(++Buffer) = Colors[i].B;
			*(++Buffer) = 1.0f;
		}

		if (ActiveAttributes&TEXCOORD0)
		{
			*(++Buffer) = Texcoord0[i].x;
			*(++Buffer) = Texcoord0[i].y;
			*(++Buffer) = Texcoord0[i].z;
		}
		else
		{
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
		}

		if (ActiveAttributes&TEXCOORD1)
		{
			*(++Buffer) = Texcoord1[i].x;
			*(++Buffer) = Texcoord1[i].y;
			*(++Buffer) = Texcoord1[i].z;
		}
		else
		{
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
		}

		if (ActiveAttributes&TEXCOORD2)
		{
			*(++Buffer) = Texcoord2[i].x;
			*(++Buffer) = Texcoord2[i].y;
			*(++Buffer) = Texcoord2[i].z;
		}
		else
		{
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
		}

		if (ActiveAttributes & TEXCOORD3)
		{
			*(++Buffer) = Texcoord3[i].x;
			*(++Buffer) = Texcoord3[i].y;
			*(++Buffer) = Texcoord3[i].z;
		}
		else
		{
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
			*(++Buffer) = 0.0f;
		}

		if (ActiveAttributes & JOINTS)
		{
			for (int j = 0; j < JOINTCOUNT; j++)
				*(++Buffer) = (float)Weights[i].ids[j];
			for (int j = 0; j < JOINTCOUNT; j++)
				*(++Buffer) = Weights[i].weights[j];
		}
	}

	assert(((long)++Buffer - (long)ByteBuf) == BufferSize);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, ByteBuf, GL_STATIC_DRAW);

	delete[] ByteBuf;

	GLuint Offset = 0;
	GLuint Index = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	//position
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index++, 4, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
	Offset += 4 * sizeof(float);
	//normal
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index++, 4, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
	Offset += 4 * sizeof(float);
	//color
	if (ActiveAttributes & COLOR)
	{
		glEnableVertexAttribArray(Index);
		glVertexAttribPointer(Index++, 4, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
		Offset += 4 * sizeof(float);
	}
	//texcoord0
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index++, 3, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
	Offset += 3 * sizeof(float);
	//texcoord1
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index++, 3, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
	Offset += 3 * sizeof(float);
	//texcoord2
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index++, 3, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
	Offset += 3 * sizeof(float);
	//texcoord3
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index++, 3, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
	Offset += 3 * sizeof(float);

	if (ActiveAttributes & JOINTS)
	{
		int vectorCount = (JOINTCOUNT + JOINTSIZE - 1) / JOINTSIZE;
		for(int i = 0; i < vectorCount; i++)
		{
			int vectorSize = std::min(JOINTCOUNT - JOINTSIZE * i, JOINTSIZE);
			glEnableVertexAttribArray(Index);
			glVertexAttribPointer(Index++, vectorSize, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
			Offset += vectorSize * sizeof(float);
		}

		for (int i = 0; i < vectorCount; i++)
		{
			int vectorSize = std::min(JOINTCOUNT - JOINTSIZE * i, JOINTSIZE);
			glEnableVertexAttribArray(Index);
			glVertexAttribPointer(Index++, vectorSize, GL_FLOAT, GL_FALSE, ElementSize, BUFFER_OFFSET(Offset));
			Offset += vectorSize * sizeof(float);
		}
	}

	BuffersInitialized = true;

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);    
}

void VertexBuffer::activate()
{
    if(!BuffersInitialized)
    {
        std::cout << "VertexBuffer::activate(): The vertex buffer is not correctly initialized!\n";
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindVertexArray(VAO);
    
}

void VertexBuffer::deactivate()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);    
}
