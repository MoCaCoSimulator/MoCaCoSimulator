#ifndef __SimpleRayTracer__color__
#define __SimpleRayTracer__color__

#include <iostream>

class Color
{
public:
    float R;
    float G;
    float B;
    
	Color() :R(0), G(0), B(0) {};
    Color( float r, float g, float b):R(r), G(g), B(b) {};
    Color operator*(const Color& c) const;
    Color operator*(const float Factor) const;
    Color operator+(const Color& c) const;
    Color& operator+=(const Color& c);
	void print() const;
	char* c_str() const;

	static Color white;
	static Color red;
	static Color green;
	static Color blue;
	static Color yellow;
	static Color turquoise;
	static Color purple;
	static Color orange;
	static Color black;
};


#endif /* defined(__SimpleRayTracer__color__) */
