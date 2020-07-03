
#include "BaseShader.h"
#include "ShaderLightMapper.h"
#include "qdebug.h"

const BaseShader* BaseShader::ShaderInPipe = NULL;

BaseShader::BaseShader() : 
	LightUniformBuffer(GL_INVALID_INDEX), 
	GlobalDiffuseColor(1.0f, 1.0f, 1.0f),
	Alpha(1.0f),
	DisableZTest(false)
{
    ModelTransform.setIdentity();
}

BaseShader::~BaseShader()
{
	printf("delete shaders %d %d\n", VS, FS);
	glDetachShader(ShaderProgram, VS);
	glDetachShader(ShaderProgram, FS);
	glDeleteProgram(ShaderProgram);
}

bool BaseShader::load( const std::string& VertexShaderFile, const std::string& FragmentShaderFile )
{
    unsigned int VSFileSize=0;
    unsigned int FSFileSize=0;
    char* VSFileData = NULL;
    char* FSFileData = NULL;
    
    VSFileData = loadFile(VertexShaderFile, VSFileSize);
    if( !VSFileData)
    {
        std::cout << "Unable to load shader file " << VertexShaderFile.c_str() << std::endl;
        return false;
    }
    
    FSFileData = loadFile(FragmentShaderFile, FSFileSize);
    if( !FSFileData)
    {
        std::cout << "Unable to load shader file " << FragmentShaderFile.c_str() << std::endl;
        return false;
    }
    
    ShaderProgram = createShaderProgram(VSFileData, FSFileData);
    
    delete [] VSFileData;
    delete [] FSFileData;
  
    return true;
}

GLuint BaseShader::createShaderProgram( const char* VScode, const char* FScode )
{
    ModelTransform.setIdentity();
    const unsigned int LogSize = 64*1024;
    char ShaderLog[LogSize];
    GLsizei WrittenToLog=0;
    GLint Success = 0;
    
    VS = glCreateShader(GL_VERTEX_SHADER);
    FS = glCreateShader(GL_FRAGMENT_SHADER);
	printf("created shaders %d %d\n", VS, FS);
    
    GLenum Error = glGetError();
    if(Error!=0)
    {
        std::cout << "OpenGL error: " << Error;
		throw std::exception();
    }
    glShaderSource(VS, 1, &VScode, NULL);
    glShaderSource(FS, 1, &FScode, NULL);
    
    glCompileShader(VS);
    glGetShaderiv(VS, GL_COMPILE_STATUS, &Success);
    if(Success==GL_FALSE)
    {
        sprintf(ShaderLog, "VS:");
        WrittenToLog+=3;
        GLsizei Written=0;
        glGetShaderInfoLog(VS, LogSize-WrittenToLog, &Written, &ShaderLog[WrittenToLog]);
        WrittenToLog+=Written;
    }

    glCompileShader(FS);
    glGetShaderiv(FS, GL_COMPILE_STATUS, &Success);
    if(Success==GL_FALSE)
    {
        sprintf(&ShaderLog[WrittenToLog], "FS:");
        WrittenToLog+=3;
        GLsizei Written=0;
        glGetShaderInfoLog(FS, LogSize-WrittenToLog, &Written, &ShaderLog[WrittenToLog]);
        WrittenToLog+=Written;
    }
    
    if( WrittenToLog > 0 )
    {
        // compilation failed
        qDebug() << ShaderLog;
        throw std::exception();
    }
    
    ShaderProgram = glCreateProgram();
    assert(ShaderProgram);
    
    glAttachShader(ShaderProgram, VS);
    glDeleteShader(VS);
    glAttachShader(ShaderProgram, FS);
    glDeleteShader(FS);
    glLinkProgram(ShaderProgram);
    
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if(Success==GL_FALSE)
    {
        GLsizei Written=0;
        glGetProgramInfoLog(ShaderProgram, LogSize-WrittenToLog, &Written, &ShaderLog[WrittenToLog]);
        WrittenToLog+=Written;
    }

    if( WrittenToLog > 0 )
    {
        // compilation failed
        std::cout << ShaderLog;
		throw std::exception();
    }
	
	LightUniformBuffer = getBlockID("Lights");
    return ShaderProgram;
}

char* BaseShader::loadFile( const std::string& File, unsigned int& Filesize )
{
    FILE* pFile = fopen(File.c_str(), "rb" );
    if(!pFile)
    {
        printf( "Unable to open file %s", File.c_str());
        return NULL;
    }
    
    fseek( pFile, 0, SEEK_END);
    Filesize = (unsigned int)ftell(pFile);
    fseek( pFile, 0, SEEK_SET);
    
    if(Filesize<=0)
        return NULL;
    
    char* pBuf = new char[Filesize+1];
    fread( pBuf, Filesize, 1, pFile);
    fclose(pFile);
    pBuf[Filesize] = 0;
    return pBuf;
    
}

void BaseShader::activate(const BaseCamera& Cam) const
{
	if (ShaderInPipe != this)
	{
		glUseProgram(ShaderProgram);
		if (LightUniformBuffer != GL_INVALID_INDEX)
			setBlock(LightUniformBuffer, ShaderLightMapper::instance().uniformBlockID());
	}
    ShaderInPipe = this;
}

void BaseShader::deactivate() const
{
    glUseProgram(0);
	ShaderInPipe = NULL;
}

GLuint BaseShader::getBlockID(const char* BlockName) const
{
	return glGetUniformBlockIndex(ShaderProgram, BlockName);
}

void BaseShader::setBlock(GLuint ID, GLuint UniformBufferID) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, ID, UniformBufferID);
}


GLint BaseShader::getParameterID(const char* ParamenterName) const
{
    return glGetUniformLocation(ShaderProgram, ParamenterName);
}

void BaseShader::setParameter( GLint ID, float Param) const
{
	if (ID < 0)
		return;
    glUniform1f(ID, Param);
}
void BaseShader::setParameter( GLint ID, int Param) const
{
	if (ID < 0)
		return;
    glUniform1i(ID, Param);
}
void BaseShader::setParameter( GLint ID, const Vector3& Param) const
{
	if (ID < 0)
		return;
    glUniform3f(ID, Param.x, Param.y, Param.z);
}
void BaseShader::setParameter( GLint ID, const Color& Param) const
{
	if (ID < 0)
		return;
    glUniform3f(ID, Param.R, Param.G, Param.B);
}

void BaseShader::setParameter( GLint ID, const Matrix& Param) const
{
	if (ID < 0)
		return;
    glUniformMatrix4fv(ID, 1, GL_FALSE, Param.m);
}

void BaseShader::color(const Color& col)
{
	GlobalDiffuseColor = col;
}

void BaseShader::alpha(const float& alpha)
{
	Alpha = alpha;
}