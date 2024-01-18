#include "pch.hpp"
#include "utils.hpp"

struct Shader
{

    bool Load(const char *vsFileName, const char *fsFileName); 
    bool Create(const char *vsCode, const char *fsCode);

    void Set();
    void Reset();

    unsigned int id;
};


struct Texture2D
{
    Texture2D()
    {
        id = 0;
        width = 0;
        height = 0;
        format = PixelFormat::R8G8B8A8;
    }
    ~Texture2D()
    {
        Release();
    }

    bool Load(const char *fileName);
    bool LoadFromMemory(const unsigned char *fileData, int dataSize);

    void Release();

    unsigned int id;        
    int width;              
    int height;              
    PixelFormat format;             
};


// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
struct BatchBuffer 
{
    int elementCount;           // Number of elements in the buffer (QUADS)
    std::vector<float>  vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    std::vector<float>  texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    std::vector<unsigned char> colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    std::vector<unsigned int>  indices;    // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
    unsigned int vaoId;         // OpenGL Vertex Array Object id
    unsigned int vboId[4];      // OpenGL Vertex Buffer Objects id (4 types of vertex data)
} ;

struct DrawCall 
{
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertexCount;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    unsigned int textureId;     // Texture id to be used on the draw -> Use to create new draw call if changes
};


struct RenderBatch 
{
 
    RenderBatch();
    ~RenderBatch();

    void Init(int numBuffers, int bufferElements);
    void Release();


    void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, const Color &color);
    
    void DrawCircleSector(const Vector2 &center, float radius, float startAngle, float endAngle, int segments, const Color &color);
    void DrawCircleSectorLines(const Vector2 &center, float radius, float startAngle, float endAngle, int segments, const Color &color);

    void DrawCircleGradient(int centerX, int centerY, float radius, const Color &color1, const Color &color2);

    void DrawCircle(int centerX, int centerY, float radius, const Color &color);
    void DrawCircleV(const Vector2 &center, float radius, const Color &color);

    
    void DrawCircleLines(int centerX, int centerY, float radius, const Color &color);
    void DrawCircleLinesV(const Vector2 &center, float radius, const Color &color);
    void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, const Color &color);
    void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, const Color &color);
    void DrawRing(const Vector2 &center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, const Color &color);
    void DrawRingLines(const Vector2 &center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, const Color &color);
    void DrawRectangle(int posX, int posY, int width, int height, const Color &color);
    void DrawRectangleV(const Vector2 &position, const Vector2 &size, const Color &color);
    void DrawRectangleRec(const Rectangle &rec, const Color &color);
    void DrawRectanglePro(const Rectangle &rec, const Vector2 &origin, float rotation, const Color &color);
    void DrawRectangleLines(int posX, int posY, int width, int height, const Color &color);
    void DrawRectangleLinesEx(const Rectangle &rec, float lineThick, const Color &color);

    void DrawTexture(Texture2D &texture, int posX, int posY, const Color &tint);
    void DrawTextureV(Texture2D &texture, const Vector2 &position, const Color &tint);
    void DrawTextureEx(Texture2D &texture,const  Vector2 &position, float rotation, float scale, const Color &tint);
    void DrawTextureRec(Texture2D &texture,  Rectangle &source, const Vector2 &position, const Color &tint);
    void DrawTexturePro(Texture2D &texture,  Rectangle &source, const Rectangle &dest, const Vector2 &origin, float rotation, const Color &tint);

    //3d 

    void DrawGrid(int slices, float spacing);
    void DrawLine3D(const Vector3 &startPos,const Vector3 &endPos, const Color &color);
    void DrawLine3D(float x, float y, float z, float x2,float y2, float z2, const Color &color);
    void DrawCube(const Vector3 &position, float width, float height, float length,const Color &color);
    void DrawCubeLines(const Vector3 &position, float width, float height, float length, const Color &color);
    void DrawTriangle(const Vector3 &v1, const Vector3 &v2,const Vector3 &v3, const Color &color);
    void DrawPlane(const Vector3 &centerPos, const Vector2 &size, const Color &color);

    void DrawBoundigBox(const BoundingBox &box, const Color &color);




    void Render();

    void Begin(int mode);                        
    void End(void);          

    void Vertex2i(int x, int y);                 
    void Vertex2f(float x, float y);          
    void Vertex3f(float x, float y, float z);     
    void TexCoord2f(float x, float y);          

    void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);  
    void Color3f(float x, float y, float z);          
    void Color4f(float x, float y, float z, float w); 

    void setProjection(const float *proj);


    private:
        bool CheckRenderBatchLimit(int vCount);
        void SetTexture(unsigned int id);

    int bufferCount;            // Number of vertex buffers (multi-buffering support)
    int currentBuffer;          // Current buffer tracking in case of multi-buffering
    int drawCounter;            // Draw calls counter
    float currentDepth;         // Current depth value for next draw
    int vertexCounter;
    unsigned int defaultTextureId;  
    unsigned int defaultShaderId;

    unsigned int mpvId;
    unsigned int textId;

    std::vector<DrawCall*> draws;
    std::vector<BatchBuffer*> vertexBuffer;

    float texcoordx, texcoordy;         
    unsigned char colorr, colorg, colorb, colora;

    float proj[16];   


    

};


class Ref
{
	public:
		Ref(): ReferenceCounter(1){		}
		virtual ~Ref()		{		}


		bool Release()
		{
			--ReferenceCounter;
			if (!ReferenceCounter)
			{
				delete this;
				return true;
			}
			return false;
		}
	private:
		UINT	ReferenceCounter;
	};


class VertexBuffer : public Ref
{
public:
    VertexBuffer(UINT size, DWORD fvf);
    int Lock(UINT offsetToLock, UINT sizeToLock, void **ppbData);
    int Unlock();

	void Use();


protected:
    ~VertexBuffer();
private:
    UINT m_size;
    DWORD m_fvf;
    GLuint m_vbo;
	GLuint m_vao;
    void* m_data;
    bool m_isLocked;
};
