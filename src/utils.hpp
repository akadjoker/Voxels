/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djoker <djoker@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/14 16:51:37 by lrosa-do          #+#    #+#             */
/*   Updated: 2024/01/18 19:00:28 by djoker           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "pch.hpp"


#define GETCWD getcwd
#define CHDIR chdir

#define MAX_DIRECTORY_FILES     512
#define MAX_FILEPATH_LENGTH      512  
#define MAX_TEXT_BUFFER_LENGTH              1024        // Size of internal static buffers used on some functions:
                                                        // TextFormat();, TextSubtext();, TextToUpper();, TextToLower();, TextToPascal();, TextSplit();
#define MAX_TEXT_UNICODE_CHARS               512        // Maximum number of unicode codepoints: GetCodepoints();
#define MAX_TEXTSPLIT_COUNT                  128        // Maximum number of substrings to split: TextSplit();


#define GLSL(src); "#version 320 es\n" #src



#define CONSOLE_COLOR_RESET "\033[0m"
#define CONSOLE_COLOR_GREEN "\033[1;32m"
#define CONSOLE_COLOR_RED "\033[1;31m"
#define CONSOLE_COLOR_PURPLE "\033[1;35m"
#define CONSOLE_COLOR_CYAN "\033[0;36m"


#define SAFE_DELETE(x) \
    { \
        delete x; \
        x = NULL; \
    }


#define SAFE_DELETE_ARRAY(x) \
    { \
        delete[] x; \
        x = NULL; \
    }


typedef unsigned int UINT;
typedef unsigned int DWORD;
typedef void *LPVOID;
typedef int   HRESULT;

#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x004
#define D3DFVF_NORMAL 0x010
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_SPECULAR 0x080

#define D3DFVF_TEX0 0x100
#define D3DFVF_TEX1 0x200
#define D3DFVF_TEX2 0x300
#define D3DFVF_TEX3 0x400


#define PI 3.14159265358979323846f
#define EPSILON 0.000001f
#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)

enum PixelFormat
{
    GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
    GRAY_ALPHA,        // 8*2 bpp (2 channels)
    R8G8B8,            // 24 bpp
    R8G8B8A8,          // 32 bpp    
};


#define LINES                                0x0001     
#define TRIANGLES                            0x0004      
#define QUADS                                0x0008  

struct Vector2
{

    Vector2()
    {
        x = 0;
        y = 0;
    }
    Vector2(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
    void set(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
    float x, y;
};

struct Vector3
{
    Vector3()
    {
        x = 0;
        y = 0;
        z = 0;
    }
    Vector3(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    void set(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    float Distance(const Vector3& v) const 
    {
       return sqrt( (v.x-x)*(v.x-x) + (v.y-y)*(v.y-y) + (v.z-z)*(v.z-z) );
    }   
    bool operator==(const Vector3& other) const 
    {
        return x == other.x && y == other.y && z == other.z;
    }

    /*
      Vector3 middle =(min + max) / 2;
			Vector3 diag = middle - max;
    */

    Vector3 operator+(const Vector3& other) const 
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const 
    {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(const Vector3& other) const 
    {
        return Vector3(x * other.x, y * other.y, z * other.z);
    }

    Vector3 operator/(const Vector3& other) const 
    {
        return Vector3(x / other.x, y / other.y, z / other.z);
    }

    Vector3 operator*(float other) const 
    {
        return Vector3(x * other, y * other, z * other);
    }

    Vector3 operator/(float other) const 
    {
        return Vector3(x / other, y / other, z / other);
    }
    
    
    float x, y, z;
};

struct Vector3Hash 
{
    size_t operator()(const Vector3& v) const 
    {
        size_t hashX = std::hash<float>()(v.x);
        size_t hashY = std::hash<float>()(v.y);
        size_t hashZ = std::hash<float>()(v.z);
        return hashX ^ (hashY << 1) ^ (hashZ << 2);
    }
};

struct Rectangle
{
    Rectangle()
    {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
    }
    Rectangle(float _x, float _y, float _width, float _height)
    {
        x = _x;
        y = _y;
        width = _width;
        height = _height;
    }
    float x, y, width, height;
};

struct Color 
{
    Color()
    {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
    }
    Color (unsigned char v)
    {
        r = v;
        g = v;
        b = v;
        a = 255;
    }
    Color (unsigned char _r, unsigned char _g, unsigned char _b)
    {
        r = _r;
        g = _g;
        b = _b;
        a = 255;
    }
    Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
    {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }

    Color(const Color& other)
    {
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
    }

    Color& operator=(const Color& other) 
    {
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
        return *this;
    }

    unsigned char r, g, b, a;


};


struct BoundingBox
{
    Vector3 min;
    Vector3 max;

    BoundingBox()
    {
            min.x=-1;
            min.y=-1;
            min.z=-1;
            max.x=1;
            max.y=1;
            max.z=1;
    }

    void addInternalPoint(float x, float y, float z)
        {
            if (x < min.x)
                min.x = x;
            if (y < min.y)
                min.y = y;
            if (z < min.z)
                min.z = z;
            if (x > max.x)
                max.x = x;
            if (y > max.y)
                max.y = y;
            if (z > max.z)
                max.z = z;
        }
        void addInternalPoint(const Vector3 &point)
        {
            if (point.x < min.x)
                min.x = point.x;
            if (point.y < min.y)
                min.y = point.y;
            if (point.z < min.z)
                min.z = point.z;
            if (point.x > max.x)
                max.x = point.x;
            if (point.y > max.y)
                max.y = point.y;
            if (point.z > max.z)
                max.z = point.z;
        }

       Vector3 getCenter() const
        {
            return (min + max) / 2;
        }

        Vector3 getHalfSize() const
        {
            return (max - min) / 2;
        }

        Vector3 getExtend() const
        {
            return (max - min);
        }
         void getEdges(Vector3 *edges) const
         {
            Vector3 middle =(min + max) / 2;
			Vector3 diag = middle - max;
            edges[0].set(middle.x + diag.x, middle.y + diag.y, middle.z + diag.z);
			edges[1].set(middle.x + diag.x, middle.y - diag.y, middle.z + diag.z);
			edges[2].set(middle.x + diag.x, middle.y + diag.y, middle.z - diag.z);
			edges[3].set(middle.x + diag.x, middle.y - diag.y, middle.z - diag.z);
			edges[4].set(middle.x - diag.x, middle.y + diag.y, middle.z + diag.z);
			edges[5].set(middle.x - diag.x, middle.y - diag.y, middle.z + diag.z);
			edges[6].set(middle.x - diag.x, middle.y + diag.y, middle.z - diag.z);
			edges[7].set(middle.x - diag.x, middle.y - diag.y, middle.z - diag.z);
         }
    
};

class Frustum
{
    public:

        void CalculateFrustum(const float *view, const float *proj);
        
        bool PointInView(float x, float y, float z);
        
        bool CubeInView( const Vector3 &b_min, const Vector3 &b_max  );
        bool CubeInView( float x, float y, float z, float size );

        bool SphereInView(float x, float y, float z, float radius);
        
        bool BoundingBoxInView(const BoundingBox &box);
        
        
        


    private:
        void NormalizePlane(int side);
        
      float m_Frustum[6][4];
};


void Log(int severity, const char* fmt, ...);

std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters);
bool find_word(const std::string& haystack,const std::string& needle);
unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead);
bool SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite);
char *LoadFileText(const char *fileName);
bool SaveFileText(const char *fileName, char *text);



//----------------------------------------------------------------------------------
// Text strings management functions
//----------------------------------------------------------------------------------
const char *strprbrk(const char *s, const char *charset);
unsigned int TextLength(const char *text);
const char *TextFormat(const char *text, ...);
int TextToInteger(const char *text);
int TextCopy(char *dst, const char *src);
bool TextIsEqual(const char *text1, const char *text2);
const char *TextSubtext(const char *text, int position, int length);
char *TextReplace(char *text, const char *replace, const char *by);
char *TextInsert(const char *text, const char *insert, int position);
const char *TextJoin(const char **textList, int count, const char *delimiter);
const char **TextSplit(const char *text, char delimiter, int *count);
void TextAppend(char *text, const char *append, int *position);
int TextFindIndex(const char *text, const char *find);
const char *TextToUpper(const char *text);
const char *TextToLower(const char *text);
const char *TextToPascal(const char *text);


const char *CodepointToUTF8(int codepoint, int *utf8Size);
int GetCodepoint(const char *text, int *codepointSize);
int GetCodepointNext(const char *text, int *codepointSize);
int GetCodepointPrevious(const char *text, int *codepointSize);
int GetCodepointCount(const char *text);
//----------------------------------------------------------------------------------
// Files management functions
//----------------------------------------------------------------------------------

bool FileExists(const char *fileName);
bool DirectoryExists(const char *dirPath);
const char *GetFileExtension(const char *fileName);
const char *GetFileName(const char *filePath);
const char *GetFileNameWithoutExt(const char *filePath);
const char *GetDirectoryPath(const char *filePath);
const char *GetPrevDirectoryPath(const char *dirPath);
const char *GetWorkingDirectory(void);
void ClearDirectoryFiles(void);
char **GetDirectoryFiles(const char *dirPath, int *fileCount);
bool ChangeDirectory(const char *dir);
bool IsFileExtension(const char *fileName, const char *ext);

void Random_Seed(const int seed);
int Random_Int(const int min, const int max);
float Random_Float(const float min, const float max);

unsigned int CompileShader(const char *shaderCode, int type);
unsigned int LoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId);
unsigned int LoadTexture(const unsigned char *data, int width, int height, PixelFormat format);
unsigned int LoadImageToGL(const char *fileName);
unsigned int LoadTextureArray(const unsigned char *data,int atlasWidth, int atlasHeight, int gridWidth , int gridHeight, PixelFormat format);
unsigned int LoadImageToGLArray(const char *fileName, int gridWidth , int gridHeight);

void UnloadVertexArray(unsigned int vaoId);
void UnloadVertexBuffer(unsigned int vboId);
void UnloadTexture(unsigned int id);
