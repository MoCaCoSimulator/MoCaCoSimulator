#include "color.h"
#include <assert.h>

Color Color::white = Color(1.0f, 1.0f, 1.0f);
Color Color::red = Color(1.0f, 0.0, 0.0);
Color Color::green = Color(0.0, 1.0f, 0.0);
Color Color::blue = Color(0.0, 0.0, 1.0f);
Color Color::yellow = Color(1.0f, 1.0f, 0.0);
Color Color::turquoise = Color(0.0, 1.0f, 1.0f);
Color Color::purple = Color(1.0f, 0.0f, 1.0f);
Color Color::orange = Color(1.0f, 0.5f, 0.0f);
Color Color::black = Color(0.0, 0.0, 0.0);

Color Color::operator*(const Color& c) const
{
	Color col(this->R * c.R, this->G * c.G, this->B * c.B);
    return col;
}

Color Color::operator*(const float Factor) const
{
	Color col(this->R * Factor, this->G * Factor, this->B * Factor);
	return col;
}

Color Color::operator+(const Color& c) const
{
	Color col(this->R + c.R, this->G + c.G, this->B + c.B);
	return col;
}

Color& Color::operator+=(const Color& c)
{
	*this = *this + c;
	return *this;
}

void Color::print() const
{
	printf("R: %f G: %f B: %f\n", this->R, this->G, this->B);
}

char * Color::c_str() const
{
	return "not implemented";
}
