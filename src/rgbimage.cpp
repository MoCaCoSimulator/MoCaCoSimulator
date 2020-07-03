#include "rgbimage.h"
#include "color.h"
#include "assert.h"
#include "vector.h"
#include "Matrix.h"
#include "math.h"

RGBImage::RGBImage( unsigned int Width, unsigned int Height)
{
	this->m_Width = Width;
	this->m_Height = Height;
	std::cout << Width << " " << Height << std::endl;
	this->m_Image = new Color[Width*Height];
}

RGBImage::~RGBImage()
{
	delete[] this->m_Image;
}

unsigned int RGBImage::getPos(unsigned int x, unsigned int y) const{
	return x + y * this->m_Width;
}

void RGBImage::setPixelColor( unsigned int x, unsigned int y, const Color& c)
{
	this->m_Image[this->getPos(x, y)] = c;
}

const Color& RGBImage::getPixelColor( unsigned int x, unsigned int y) const
{
	return this->m_Image[this->getPos(x, y)];
}

unsigned int RGBImage::width() const
{
	return this->m_Width;
}
unsigned int RGBImage::height() const
{
	return this->m_Height;
}

RGBImage& RGBImage::GaussFilter(RGBImage& dst, const RGBImage& src, float factor) {
	unsigned int NumSegX = src.width();
	unsigned int NumSegY = src.height();
	assert(dst.height() == NumSegY && dst.width() == NumSegX);
	float K[7] = { 0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f };
	float* GT = new float[src.height()*src.width()];

	float G;
	for (unsigned int x = 0; x < NumSegX; ++x)
		for (unsigned int y = 0; y < NumSegY; ++y)
		{
			G = 0;
			for (int i = 0; i <= 6; i++)
				G += src.getPixelColor(clamp(x + i - 3, 0, NumSegX-1), y).G * K[i];
			GT[x * NumSegX + y] = G;
		}

	for (unsigned int x = 0; x < NumSegX; ++x)
		for (unsigned int y = 0; y < NumSegY; ++y)
		{
			G = 0;
			for (int i = 0; i <= 6; i++)
				G += GT[ x * NumSegX + clamp(y + i - 3, 0, NumSegY-1)] * K[i];
			G = clamp(G, 0.f, 1.f);
			dst.setPixelColor(x, y, Color(G, G, G));
		}

	delete[] GT;
	return dst;
}

RGBImage& RGBImage::SobelFilter(RGBImage& dst, const RGBImage& src, float factor) {
	assert(dst.height() == src.height() && dst.width() == src.width());

	int K[3][3] = {
		{ 1, 2, 1 },
		{ 0, 0, 0 },
		{ -1, -2, -1 }
	};
	int KT[3][3] = {
		{ 1, 0, -1 },
		{ 2, 0, -2 },
		{ 1, 0, -1 }
	};

	float U, V, col, power;
	for (unsigned int x = 0; x < src.width(); ++x)
		for (unsigned int y = 0; y < src.height(); ++y)
		{
			U = 0;
			V = 0;
			for (int i = 0; i <= 2; ++i)
				for (int j = 0; j <= 2; ++j)
				{
					col = src.getPixelColor(clamp(x + i - 1, 0, src.width()-1), clamp(y + j - 1, 0, src.height() - 1)).B;
					U += col * K[i][j];
					V += col * KT[i][j];
				}

			power = clamp( sqrt(U*U + V*V) * factor, 0.f, 1.f);
			dst.setPixelColor(x, y, Color(power, power, power));
		}

	return dst;
}

float RGBImage::clamp(float v, float min, float max)
{
	return ((v < max) ? ((v > min) ? v : min) : max);
}

int RGBImage::clamp(int v, int min, int max)
{
	return ((v < max) ? ((v > min) ? v : min) : max);
}

unsigned char RGBImage::convertColorChannel( float v )
{
	return (unsigned char)(clamp(v, 0.f, 1.f) * 255);
}

bool RGBImage::saveToDisk( const char* Filename)
{
	BITMAPFILEHEADER_IMAGE fheader;
	BITMAPINFOHEADER_IMAGE finfo;
	unsigned int file_size = m_Height * m_Width * 3;
	unsigned int info_size = sizeof(BITMAPFILEHEADER_IMAGE) + sizeof(BITMAPINFOHEADER_IMAGE);

	fheader.bfType = 19778; //MB for BM => Bitmap
	fheader.bfReserved = 0;
	fheader.bfSize = info_size + file_size;
	fheader.bfOffBits = info_size;

	finfo.biSize = sizeof(BITMAPINFOHEADER_IMAGE);
	finfo.biWidth = m_Width;
	finfo.biHeight = m_Height;
	finfo.biPlanes = 1;
	finfo.biBitCount = 24;
	finfo.biCompression = 0;
	finfo.biSizeImage = file_size;
	finfo.biXPelsPerMeter = 0;
	finfo.biYPelsPerMeter = 0;
	finfo.biClrUsed = 0;
	finfo.biClrImportant = 0;

	unsigned char *img = (unsigned char*)malloc(file_size);

	for (unsigned int w = 0; w < m_Width; w++) {
		for (unsigned int h = 0; h < m_Height; h++) {
			int it_m = w + h * m_Width;
			int it = ((w + (m_Height-1-h) * m_Width) * 3);
			img[it + 2] = this->convertColorChannel(m_Image[it_m].R);
			img[it + 1] = this->convertColorChannel(m_Image[it_m].G);
			img[it] = this->convertColorChannel(m_Image[it_m].B);
		}
	}

	FILE *bmp = fopen(Filename, "wb");
	if (bmp == NULL)
		return false;
	if (!(fwrite(&fheader, sizeof(BITMAPFILEHEADER_IMAGE), 1, bmp) > 0))
		return false;
	if (!(fwrite(&finfo, sizeof(BITMAPINFOHEADER_IMAGE), 1, bmp) > 0))
		return false;
	if (!(fwrite(img, sizeof(unsigned char), file_size, bmp) > 0))
		return false;

	fclose(bmp);
	return true;
}
