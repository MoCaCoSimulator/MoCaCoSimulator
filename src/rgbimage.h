#ifndef __SimpleRayTracer__rgbimage__
#define __SimpleRayTracer__rgbimage__

#include <iostream>
#include <stdint.h>
class Color;

#pragma pack(1)
typedef struct BITMAPFILEHEADER_IMAGE {
	uint16_t bfType;
	uint32_t bfSize;
	uint32_t bfReserved;
	uint32_t bfOffBits;
} BITMAPFILEHEADER_IMAGE;
#pragma pack()

typedef struct BITMAPINFOHEADER_IMAGE {
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} BITMAPINFOHEADER_IMAGE;


class RGBImage
{
public:
    RGBImage( unsigned int Width, unsigned int Height);
    ~RGBImage();
	unsigned int getPos(unsigned int x, unsigned int y) const;
    void setPixelColor( unsigned int x, unsigned int y, const Color& c);
    const Color& getPixelColor( unsigned int x, unsigned int y) const;
    bool saveToDisk( const char* Filename);
    unsigned int width() const;
    unsigned int height() const;

	static float clamp(float v, float min, float max);
	static int clamp(int v, int min, int max);
	static RGBImage& GaussFilter(RGBImage& dst, const RGBImage& src, float factor = 1.0f);
	static RGBImage& SobelFilter(RGBImage& dst, const RGBImage& src, float factor = 1.0f);
    static unsigned char convertColorChannel( float f);
protected:
    Color* m_Image;
    unsigned int m_Height;
    unsigned int m_Width;
};

#endif /* defined(__SimpleRayTracer__rgbimage__) */
