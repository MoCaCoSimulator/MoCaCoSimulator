//
//  IndexBuffer.hpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef IndexBuffer_hpp
#define IndexBuffer_hpp

#include <iostream>
#include <vector>
#include <stdio.h>
#include <GL/glew.h>

class IndexBuffer
{
public:
    IndexBuffer();
    ~IndexBuffer();
    
    void begin();
    void addIndex( unsigned int Index);
    void end();
    
    void activate();
    void deactivate();
    
    GLenum indexFormat() const { return IndexFormat; }
    unsigned int indexCount() const { return IndexCount; }
	const std::vector<unsigned int>& indices() const { return Indices; }
    
private:
    std::vector<unsigned int> Indices;

    GLuint IBO;
    bool BufferInitialized;
    bool WithinBeginAndEnd;
    
    GLenum IndexFormat;
    unsigned int IndexCount;
    
};

#endif /* IndexBuffer_hpp */
