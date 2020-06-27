//
//  VertexBuffer.hpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef VertexBuffer_hpp
#define VertexBuffer_hpp
#define JOINTCOUNT 8
#define JOINTSIZE 4

#include <iostream>
#include <vector>
#include <stdio.h>
#include <GL/glew.h>
#include "vector.h"
#include "color.h"
#include <algorithm>
#include <cassert>

class VertexBuffer
{
public:
	struct Texcoord
    {
		float s;
		float t;
		float u;

        Texcoord() : s(0), t(0), u(0) {}
        Texcoord(float _s, float _t, float _u=0) : s(_s), t(_t), u(_u) {}
    };

	struct JointWeights
	{
		int ids[JOINTCOUNT];
		float weights[JOINTCOUNT];

		JointWeights() 
		{
			for (int i = 0; i < JOINTCOUNT; i++)
			{
				ids[i] = 0;
				weights[i] = 0.0f;
			}
		}
		void AddWeight(int id, float weight)
		{
			for (int i = 0; i < JOINTCOUNT; i++)
			{
				if (weights[i] != 0.0f)
					continue;

				ids[i] = id;
				weights[i] = weight;
				return;
			}

			assert(false);
		}

		JointWeights Sorted() const
		{
			JointWeights sorted;

			float totalw = 0.0f;
			float max = std::numeric_limits<float>::max();
			for (int i = 0; i < JOINTCOUNT; i++)
			{
				float heighestWeight = 0.0f;
				int heighestIndex = 0;
				for (int j = 0; j < JOINTCOUNT; j++)
				{
					float weight = weights[j];

					if (weight <= heighestWeight || weight >= max)
						continue;

					heighestWeight = weight;
					heighestIndex = ids[j];
				}

				if (heighestWeight == 0.0f)
					break;

				max = heighestWeight;
				sorted.weights[i] = heighestWeight;
				sorted.ids[i] = heighestIndex;

				totalw += weights[i];
			}

			return sorted;
		}
	};
    
    VertexBuffer();
    ~VertexBuffer();
    
    void begin();
    void addNormal( float x, float y, float z);
    void addNormal( const Vector3& v);
    void addColor( const Color& c);
    void addTexcoord0( float s, float t, float u=0.0f );
    void addTexcoord1( float s, float t, float u=0.0f );
    void addTexcoord2( float s, float t, float u=0.0f );
    void addTexcoord3( float s, float t, float u=0.0f );
    void addVertex( float x, float y, float z);
    void addVertex( const Vector3& v);
	void addJointWeights( const JointWeights& jointWeights);
    void end();
    
    void activate();
    void deactivate();
    
    unsigned int vertexCount() const { return VertexCount; }
    
    const std::vector<Vector3>& vertices() const { return Vertices; }
    const std::vector<Vector3>& normals() const { return Vertices; }
    const std::vector<Color>& colors() const { return Colors; }
    const std::vector<Vector3>& texcoord0() const { return Texcoord0; }
    const std::vector<Vector3>& texcoord1() const { return Texcoord1; }
    const std::vector<Vector3>& texcoord2() const { return Texcoord2; }
    const std::vector<Vector3>& texcoord3() const { return Texcoord3; }
	const std::vector<JointWeights>& jointWeights() const { return Weights; }

private:
    enum ATTRIBUTES
    {
        VERTEX  = 1<<0,
        NORMAL  = 1<<1,
        COLOR   = 1<<2,
        TEXCOORD0 = 1<<3,
        TEXCOORD1 = 1<<4,
        TEXCOORD2 = 1<<5,
		TEXCOORD3 = 1 << 6,
		JOINTS = 1 << 7,
    };
    std::vector<Vector3> Vertices;
    std::vector<Vector3> Normals;
    std::vector<Color> Colors;
    std::vector<Vector3> Texcoord0;
    std::vector<Vector3> Texcoord1;
    std::vector<Vector3> Texcoord2;
    std::vector<Vector3> Texcoord3;
	std::vector<JointWeights> Weights;
    unsigned int ActiveAttributes;
    bool WithinBeginBlock;
    GLuint VBO;
    GLuint VAO;
    bool BuffersInitialized;
    unsigned int VertexCount;
    
    
};

#endif /* VertexBuffer_hpp */
