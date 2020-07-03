
#ifndef __RealtimeRending__Texture__
#define __RealtimeRending__Texture__

#include <iostream>
#include <map>
#include <string>
#include <GL/glew.h>

class RGBImage;

class Texture
{
public:
    Texture();
    Texture(unsigned int width, unsigned int height, unsigned char* data);
	Texture(unsigned int width, unsigned int height, GLint InternalFormat, GLint Format, GLint ComponentSize, GLint MinFilter, GLint MagFilter, GLint AddressMode, bool GenMipMaps);
    Texture(const char* Filename );
    Texture(const RGBImage& img);
    ~Texture();
    bool load(const char* Filename);
    bool create(unsigned int width, unsigned int height, unsigned char* data);
	bool create(unsigned int width, unsigned int height, GLint InternalFormat, GLint Format, GLint ComponentSize, GLint MinFilter, GLint MagFilter, GLint AddressMode, bool GenMipMaps);
    bool create(const RGBImage& img);
    void activate(int slot=0) const;
    void deactivate() const;
    bool isValid() const;
	unsigned int width() const;
	unsigned int height() const;
	GLuint ID() const;
    const RGBImage* getRGBImage() const;
    static Texture* defaultTex();
	static Texture* defaultNormalTex();
    static const Texture* LoadShared(const char* Filename);
    static void ReleaseShared( const Texture* pTex );
    
protected:
    void release();
    RGBImage* createImage( unsigned char* Data, unsigned int width, unsigned int height );
    GLuint m_TextureID;
    RGBImage* m_pImage;
	unsigned int Width;
	unsigned int Height;
    mutable int CurrentTextureUnit;
    static Texture* pDefaultTex;
	static Texture* pDefaultNormalTex;
    
    struct TexEntry
    {
        int RefCount;
        const Texture* pTex;
    };
    typedef std::map<std::string, TexEntry> SharedTexMap;
    static SharedTexMap SharedTextures;
	int TexID;
	std::string Name;
};

#endif /* defined(__RealtimeRending__Texture__) */
