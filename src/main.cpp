#include <iostream>
#include "Batch.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

#include "Noise.hpp"
#include "FastNoiseLite.h"






unsigned int SCR_WIDTH = 1024;
unsigned int SCR_HEIGHT = 720;
const float MAX_DISTANCE = 1500.0f;

#define MAX_BATCH_ELEMENTS  8192

Color fogDefaul(0.2*255,0.2*255,0.2*255);
Color fogWater(0.5*255,0.7*255,1.0*255);

float fogStart = 200.0;
float fogEnd = 1000.0;
float fogDensity = 0.01;

Color fogColor(0.2*255,0.2*255,0.2*255);
FastNoiseLite noise;


float scale1 = 0.2f;
float scale2 = 0.267f;
float scale3 = 0.130f;
int seed = 1337;



SDL_Window *window;
SDL_GLContext context;
Texture2D texture;
RenderBatch batch;
RenderBatch batch3D;
Camera camera(glm::vec3(0.0f, 30.0f, 10.0f));
Frustum frustum;
Vector2 mousePosition;
int mouseButton;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true; 

bool m_shouldclose;
void Wait(float ms)
{
SDL_Delay((int)ms);
}
Uint32 GetTicks(void)
{
    return SDL_GetTicks();
}
double GetTime(void)
{
    return (double) SDL_GetTicks()/1000.0;
}
void Swap()
{
    SDL_GL_SwapWindow(window);
}

Vector2 GetMousePosition()
{
    return mousePosition;
} 

bool IsMouseDown(int button)
{
    return mouseButton & SDL_BUTTON(button);
}

void MouseUpdate()
{
    int x,y;
    mouseButton = SDL_GetMouseState(&x,&y) ;
    mousePosition.set(x,y);
}



void ResizeWindow(int width,int height)
{
    if (height == 0) height = 1;
    SCR_WIDTH = width;
    SCR_HEIGHT = height;

    glViewport(0, 0, width, height);
    float L=0.0f;
    float R= width;
    float T=0.0f;
    float B= height;

    const float ortho_projection[16] =
    {
            2.0f/(R-L),   0.0f,         0.0f,   0.0f ,
            0.0f,         2.0f/(T-B),   0.0f,   0.0f ,
            0.0f,         0.0f,        -1.0f,   0.0f ,
            (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f ,
    };
    batch.setProjection(ortho_projection);
    }


bool Run()
{


    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        
        switch (event.type)
        {
            case SDL_QUIT:
            {
                m_shouldclose = true;
                break;
            }
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        ResizeWindow(event.window.data1,event.window.data2);
                        break;
                    }
                }
                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym==SDLK_ESCAPE)
                {
                    m_shouldclose = true;
                    break;
                }
        
                break;
            }
            
            case SDL_KEYUP:
            {
            
            }
            break;
            case SDL_MOUSEBUTTONDOWN:
            {
        

            }break;
            case SDL_MOUSEBUTTONUP:
            {
                
                break;
            }
            case SDL_MOUSEMOTION:
            {

            break;   
            }
            
            case SDL_MOUSEWHEEL:
            {
            
            break;
            }
        }
    } 
    MouseUpdate();
    return !m_shouldclose;
}


const int CHUNK_SIZE = 32;
const int SIZE = 128;// * CHUNK_SIZE;
const int CHUNK_HEIGHT = 256;

Noise perlinNoise(1337);
struct SkyBox 
{
    void Init()
    {
        const char *defaultVShaderCode =R"(
            #version 320 es                       
            precision mediump float;           
            layout (location = 0) in vec3 aPos;

                out vec3 TexCoords;

                uniform mat4 projection;
                uniform mat4 view;

                void main()
                {
                    TexCoords = aPos;
                    mat4 v = mat4(mat3(view));
                    gl_Position = projection * v * vec4(aPos, 1.0);
                }  

            )";

           const char *defaultFShaderCode =R"(
            #version 320 es      

                precision mediump float; 
                out vec4 FragColor;

                in vec3 TexCoords;

                uniform samplerCube skybox;
                uniform vec3 color;
                uniform bool underwater;
                uniform float colorDensity ;

                void main()
                {    
                    vec3 skyboxColor = texture(skybox, TexCoords).rgb;
                    if (underwater)
                    {
                        skyboxColor =  color;
                    } else 
                    {
                        skyboxColor = mix(skyboxColor, color, colorDensity);
                    
                    }
                    FragColor = vec4(skyboxColor, 1.0);
                }

                )";

        unsigned int vShaderId = 0;
        unsigned int fShaderId = 0;
        

        vShaderId = CompileShader(defaultVShaderCode, GL_VERTEX_SHADER);
        fShaderId = CompileShader(defaultFShaderCode, GL_FRAGMENT_SHADER);
        if (vShaderId != 0 && fShaderId != 0)
        {
            shaderProgram = LoadShaderProgram(vShaderId, fShaderId);
            if (shaderProgram != 0)
            {
                Log(0, "SHADER: [ID %i] skybox shader loaded successfully", shaderProgram);

                glDeleteShader(vShaderId);
                glDeleteShader(fShaderId);
            }
        } else 
        {
            Log(2,"SHADER: Failed to load skybox shader");
            return ;
        }

        uniformProj       = glGetUniformLocation(shaderProgram, "projection");
        uniformView       = glGetUniformLocation(shaderProgram, "view");
        uniformTexture0 = glGetUniformLocation(shaderProgram, "skybox");
        uniformColor = glGetUniformLocation(shaderProgram, "color");
        uniformUnderwater = glGetUniformLocation(shaderProgram, "underwater");
        uniformColorForce = glGetUniformLocation(shaderProgram, "colorDensity");
        float size=MAX_DISTANCE /4.0f;
        float skyboxVertices[] = 
        {
    // positions          
    -size,  size, -size,
    -size, -size, -size,
     size, -size, -size,
     size, -size, -size,
     size,  size, -size,
    -size,  size, -size,

    -size, -size,  size,
    -size, -size, -size,
    -size,  size, -size,
    -size,  size, -size,
    -size,  size,  size,
    -size, -size,  size,

     size, -size, -size,
     size, -size,  size,
     size,  size,  size,
     size,  size,  size,
     size,  size, -size,
     size, -size, -size,

    -size, -size,  size,
    -size,  size,  size,
     size,  size,  size,
     size,  size,  size,
     size, -size,  size,
    -size, -size,  size,

    -size,  size, -size,
     size,  size, -size,
     size,  size,  size,
     size,  size,  size,
    -size,  size,  size,
    -size,  size, -size,

    -size, -size, -size,
    -size, -size,  size,
     size, -size, -size,
     size, -size, -size,
    -size, -size,  size,
     size, -size,  size
};

       
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        glGenBuffers(1, &VBO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 3 * sizeof(float), 0);

        glBindVertexArray(0);
        std::vector<std::string> faces;
        faces.push_back("assets/skybox/right.jpg");
        faces.push_back("assets/skybox/left.jpg");
        faces.push_back("assets/skybox/top.jpg");
        faces.push_back("assets/skybox/bottom.jpg");
        faces.push_back("assets/skybox/front.jpg");
        faces.push_back("assets/skybox/back.jpg");


        defaultTextureId = LoadCubemap(faces);

    }
    void Free()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteTextures(1, &defaultTextureId);
    }

    void Render(const glm::mat4 &projection, const glm::mat4 &view, float dt)
    {
        
        glDepthFunc(GL_LEQUAL); 
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(uniformProj, 1, false, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, false, glm::value_ptr(view));
        glUniform3f(uniformColor, color.x, color.y,color.z);
        glUniform1i(uniformUnderwater, underwater);
        glUniform1f(uniformColorForce, colorForce);
        glUniform1i(uniformTexture0, 0);

        if (!underwater)
         {
            if (colorForce > 0.0f)
            {
                colorForce -= dt * 0.1f;
             //   Log(0,"colorForce %f",colorForce);
            }
            color.set(0.5f,0.5f,0.5f); 
         } else
         {
            color.set(fogColor.r/255.0f,fogColor.g/255.0f,fogColor.b/255.0f);
         }
   
   
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, defaultTextureId);
        glDrawArrays(GL_TRIANGLES, 0, 36);
         glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); 

    
    }


    unsigned int VBO;
    unsigned int VAO;

    unsigned int shaderProgram;
    unsigned int uniformProj;
    unsigned int uniformView;
    unsigned int uniformColor;
    unsigned int uniformUnderwater;
    unsigned int uniformColorForce;

    unsigned int uniformTexture0;
    unsigned int defaultTextureId;
    Vector3 color;
    float colorForce;
    bool underwater;
};

struct WaterPlane
{
   void Init()
    {


        const char *defaultVShaderCode =R"(
            #version 320 es                       
            precision mediump float;           
            in vec3 pos;            
            in vec4 color;            
            in vec2 uv;            

            uniform float time; 


            out vec2 fragTexCoord;             
            out vec4 fragColor;

                 
            uniform mat4 mvp;                  
            void main()                        
            {                          

                 float scrollSpeedX = 0.01;
                 float scrollSpeedY = 0.02;

                
                vec2 scrollingUV = uv + vec2(time * scrollSpeedX, time * scrollSpeedY);


                fragTexCoord = scrollingUV; 
                fragColor = color;
                gl_Position = mvp * vec4(pos , 1.0); 
       
            };)";

           const char *defaultFShaderCode =R"(
            #version 320 es      

            #define FOGMODE_EXP 1.
            #define FOGMODE_EXP2 2.
            #define FOGMODE_LINEAR 3.
            #define E 2.71828

            precision mediump float;           

           

            in vec2 fragTexCoord;              
            in vec4 fragColor;
            out vec4 finalColor;               
            uniform sampler2D texture0;    


            uniform vec4 uFogInfos;
            uniform vec3 uFogColor;


            float calcFogFactor() 
            {

                float fogDistance = gl_FragCoord.z / gl_FragCoord.w;

                float fogCoeff = 1.0;
                float fogStart = uFogInfos.y;
                float fogEnd = uFogInfos.z;
                float fogDensity = uFogInfos.w;

                if (FOGMODE_LINEAR == uFogInfos.x) {
                    fogCoeff = (fogEnd - fogDistance) / (fogEnd - fogStart);
                }
                else if (FOGMODE_EXP == uFogInfos.x) {
                    fogCoeff = 1.0 / pow(E, fogDistance * fogDensity);
                }
                else if (FOGMODE_EXP2 == uFogInfos.x) {
                    fogCoeff = 1.0 / pow(E, fogDistance * fogDistance * fogDensity * fogDensity);
                }

                return clamp(fogCoeff, 0.0, 1.0);
            } 
         
            void main()                        
            {                                  
                vec4 texelColor = texture(texture0, fragTexCoord)  * fragColor;
                vec4 fog_color = texelColor;
                float fog = calcFogFactor();
                fog_color.rgb = fog * fog_color.rgb + (1.0 - fog) * uFogColor;
                finalColor = fog_color;        
            };)";

        unsigned int vShaderId = 0;
        unsigned int fShaderId = 0;
        

        vShaderId = CompileShader(defaultVShaderCode, GL_VERTEX_SHADER);
        fShaderId = CompileShader(defaultFShaderCode, GL_FRAGMENT_SHADER);
        if (vShaderId != 0 && fShaderId != 0)
        {
            shaderProgram = LoadShaderProgram(vShaderId, fShaderId);
            if (shaderProgram != 0)
            {
                Log(0, "SHADER: [ID %i] water shader loaded successfully", shaderProgram);

                glDeleteShader(vShaderId);
                glDeleteShader(fShaderId);
            }
        } else 
        {
            Log(2,"SHADER: Failed to load water shader");
            return ;
        }



    uniformMVP       = glGetUniformLocation(shaderProgram, "mvp");
    uniformTexture0 = glGetUniformLocation(shaderProgram, "texture0");
    uniformFogColor = glGetUniformLocation(shaderProgram, "uFogColor");
    uniformFogInfos = glGetUniformLocation(shaderProgram, "uFogInfos");
    uniformTime= glGetUniformLocation(shaderProgram, "time");

    float x_tiles =8.0f;
    float y_tiles =8.0f;

    float size = SIZE * 2;

    float W =size  / x_tiles;  // Largura de cada tile em coordenadas de textura
    float H =size  / y_tiles;  // Altura de cada tile em coordenadas de textura


 float vertices[] = 
 {
       
        // positions          // colors           // texture coords
         size,  1.1f, size,   1.0f, 1.0f, 1.0f,0.4f,   W, H, // top right
         size,  1.1f, -size,   1.0f, 1.0f, 1.0f,0.4f,   W, 0.0f, // bottom right
        -size,  1.1f, -size,   1.0f, 1.0f, 1.0f,0.4f,   0.0f, 0.0f, // bottom left
        -size,  1.1f, size,   1.0f, 1.0f, 1.0f,0.4f,   0.0f, H  // top left 
    };
    unsigned int indices[] = 
    {  
        0, 1, 3, 
        1, 2, 3  
    };
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);



        glGenBuffers(1, &VBO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 9 * sizeof(float), 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, 0, 9 * sizeof(float), (void*)(3 * sizeof(float)));


        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, 0, 9 * sizeof(float), (void*)(7 * sizeof(float)));

        

        glBindVertexArray(0);

        defaultTextureId = LoadImageToGL("assets/water2.jpeg");


 

         

    }
    void Free()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

    }

    void Render(const Vector3 &camera, const glm::mat4 &projection, const glm::mat4 &view)
    {
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(camera.x,camera.y,camera.z));
        glm::mat4 mvp = projection * view * model;

        
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(uniformMVP, 1, false, glm::value_ptr(mvp));
        glUniform1i(uniformTexture0, 0);
        glUniform1f(uniformTime, GetTime());

        //x type y start z end w density
        glUniform4f(uniformFogInfos, 2.0, fogStart, fogEnd, fogDensity);
        glUniform3f(uniformFogColor, fogColor.r/255.0f, fogColor.g/255.0f,fogColor.b/255.0f);   
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, defaultTextureId);





        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

 
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);



    }


    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    unsigned int uniformTime;
    unsigned int shaderProgram;
    unsigned int uniformMVP;
    unsigned int uniformFogColor;
    unsigned int uniformFogInfos;
    unsigned int uniformTexture0;
    unsigned int defaultTextureId;
};


Vector3 voxelVerts[8] = 
{
     Vector3(0.0f, 0.0f, 0.0f),
	 Vector3(1.0f, 0.0f, 0.0f),
	 Vector3(1.0f, 1.0f, 0.0f),
	 Vector3(0.0f, 1.0f, 0.0f),
	 Vector3(0.0f, 0.0f, 1.0f),
	 Vector3(1.0f, 0.0f, 1.0f),
	 Vector3(1.0f, 1.0f, 1.0f),
	 Vector3(0.0f, 1.0f, 1.0f),
};



int voxelTris[6][4] = 
{
      	// 0 1 2 2 1 3
		{0, 3, 1, 2}, // Back Face
 		{5, 6, 4, 7}, // Front Face
	    {3, 7, 2, 6}, // Top Face
		{1, 5, 0, 4}, // Bottom Face
		{4, 7, 0, 3}, // Left Face
		{1, 2, 5, 6} // Right Face
};

Vector2 voxelUVs[4] = 
{
    Vector2 (0.0f, 1.0f),
    Vector2 (0.0f, 0.0f),
    Vector2 (1.0f, 1.0f),
    Vector2 (1.0f, 0.0f)
};

struct InstanceData
{   

    void Init()
    {


    unsigned char pixels[4] = { 255, 255, 255, 255 };   
    //defaultTextureId = LoadTexture(pixels, 1, 1, PixelFormat::R8G8B8A8);
    //defaultTextureId = LoadImageToGL("assets/default.png");

    defaultTextureId = LoadImageToGLArray("assets/atlas.png",4,4);


        const char *defaultVShaderCode =R"(
            #version 320 es                       
            precision mediump float;           
            in vec3 pos;            
            in vec2 uv;  
            in vec3 normal;          
            in vec3 position;            
            out vec2 fragTexCoord;             
            out flat int textureIndex;   
            out  vec3 fragNormal;

                 
            uniform mat4 vp;                  
            void main()                        
            {             
                fragNormal = normal;                     
                fragTexCoord = uv; 
                gl_Position = vp * vec4(pos + position, 1.0); 
                textureIndex = 0;

            if (position.y <= 0.0) 
                {
                    textureIndex = 4; // agua
                   
                } 
                else if ((position.y > 0.0) && (position.y <= 1.0))
                {
                    textureIndex = 3; // Terra lama
                   
                } 
                else if ((position.y > 1.0) && (position.y <= 12.0)) 
                {
                    textureIndex = 1; // terra com erva
                } 
                else 
                {
                    textureIndex = 2; // terra com erva
                    if (normal.y == 1.0  && normal.x == 0.0 && normal.z == 0.0)
                    {
                        textureIndex = 0; // Se a normal for para cima, use a textura da terra
                    }

                }  
       
     
            };)";


            const char *defaultFShaderCode =R"(
            #version 320 es      



            precision mediump sampler2DArray;
            precision mediump float;           


            #define FOGMODE_EXP 1.
            #define FOGMODE_EXP2 2.
            #define FOGMODE_LINEAR 3.
            #define E 2.71828

            

            in vec2 fragTexCoord;     
            in vec3 fragNormal;         
            in flat int  textureIndex;
            out vec4 finalColor;               
            uniform sampler2DArray texture0;    



            uniform vec4 uFogInfos;
            uniform vec3 uFogColor;
            uniform vec3 uAmbientColor;



            float calcFogFactor() 
            {

                float fogDistance = gl_FragCoord.z / gl_FragCoord.w;

                float fogCoeff = 1.0;
                float fogStart = uFogInfos.y;
                float fogEnd = uFogInfos.z;
                float fogDensity = uFogInfos.w;

                if (FOGMODE_LINEAR == uFogInfos.x) {
                    fogCoeff = (fogEnd - fogDistance) / (fogEnd - fogStart);
                }
                else if (FOGMODE_EXP == uFogInfos.x) {
                    fogCoeff = 1.0 / pow(E, fogDistance * fogDensity);
                }
                else if (FOGMODE_EXP2 == uFogInfos.x) {
                    fogCoeff = 1.0 / pow(E, fogDistance * fogDistance * fogDensity * fogDensity);
                }

                return clamp(fogCoeff, 0.0, 1.0);
            } 

         
         
            void main()                        
            {                                  
                vec4 texelColor = texture(texture0,  vec3(fragTexCoord.xy, textureIndex)); 
                vec4 fog_color = texelColor;


                    vec3 normal = normalize(fragNormal);
                    vec3 LightDirection = vec3(0.05, 0.6, 0.5);
                    float ambientIntensity = max(0.0, dot(normal, LightDirection));

                    vec3 ambientLight = vec3(0.6,0.6,0.6) * 0.2;


                
                float fog = calcFogFactor();
                
               // fog_color.rgb = fog * fog_color.rgb + (1.0 - fog) * uFogColor;

                fog_color.rgb = fog * (texelColor.rgb + ambientLight) + (1.0 - fog) * uFogColor;



                finalColor = fog_color;          
            };)";

    unsigned int vShaderId = 0;
    unsigned int fShaderId = 0;
    

    vShaderId = CompileShader(defaultVShaderCode, GL_VERTEX_SHADER);
    fShaderId = CompileShader(defaultFShaderCode, GL_FRAGMENT_SHADER);
    if (vShaderId != 0 && fShaderId != 0)
    {
        instanceProgram = LoadShaderProgram(vShaderId, fShaderId);
        if (instanceProgram != 0)
        {
            Log(0, "SHADER: [ID %i] Instance shader loaded successfully", instanceProgram);

             glDeleteShader(vShaderId);
             glDeleteShader(fShaderId);
        }
    } else 
    {
        Log(2,"SHADER: Failed to load Instance shader");
    }

    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    
    std::vector<Vector2> uvs;
    std::vector<unsigned int> triangles;
    int vertexIndex = 0;
    for (int p=0;p<6;p++)
    {
         

            vertices.push_back( voxelVerts[voxelTris[p][0]]);
            vertices.push_back( voxelVerts[voxelTris[p][1]]);
            vertices.push_back( voxelVerts[voxelTris[p][2]]);
            vertices.push_back( voxelVerts[voxelTris[p][3]]);
             normals.push_back(Vector3(0.0f, 0.0f, 0.0f));
              normals.push_back(Vector3(0.0f, 0.0f, 0.0f));
               normals.push_back(Vector3(0.0f, 0.0f, 0.0f));
                normals.push_back(Vector3(0.0f, 0.0f, 0.0f));

       


            uvs.push_back(Vector2(0, 1));
            uvs.push_back(Vector2(0, 0));
            uvs.push_back(Vector2(1, 1));
            uvs.push_back(Vector2(1, 0));


            triangles.push_back (vertexIndex);
            triangles.push_back (vertexIndex + 1);
            triangles.push_back (vertexIndex + 2);
            triangles.push_back (vertexIndex + 2);
            triangles.push_back (vertexIndex + 1);
            triangles.push_back (vertexIndex + 3);
            vertexIndex += 4;
            
        
    }

    // //calulate normals brute force
    // for (int i = 0; i < (int)triangles.size(); i+=3)
    // {
    //     Vector3 v0 = vertices[triangles[i]];
    //     Vector3 v1 = vertices[triangles[i+1]];
    //     Vector3 v2 = vertices[triangles[i+2]];
    //     Vector3 normal = (v1-v0).Cross(v2-v0).Normalized();
    //     normals.push_back(normal);
    //     normals.push_back(normal);
    //     normals.push_back(normal);
    // }


    //smoth normals

for (int i = 0; i < (int)triangles.size(); i+=3)
{
    Vector3 v0 = vertices[triangles[i]];
    Vector3 v1 = vertices[triangles[i+1]];
    Vector3 v2 = vertices[triangles[i+2]];
    Vector3 normal = (v1-v0).Cross(v2-v0).Normalized();
    normals[triangles[i]] += normal;
    normals[triangles[i+1]] += normal;
    normals[triangles[i+2]] += normal;
} 
for (int i = 0; i < (int)normals.size(); i++)
{
    normals[i] = normals[i].Normalized();
}

    count = triangles.size();



    uniformVP       = glGetUniformLocation(instanceProgram, "vp");
    uniformTexture0 = glGetUniformLocation(instanceProgram, "texture0");
    uniformFogColor = glGetUniformLocation(instanceProgram, "uFogColor");
    uniformFogInfos = glGetUniformLocation(instanceProgram, "uFogInfos");
  


    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glGenBuffers(5, VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()  * sizeof(Vector3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);



    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(Vector2), uvs.data() , GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);




    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vector3), normals.data() , GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);


    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(Vector3) , translations.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, 0, 0, 0);
    glVertexAttribDivisor(3, 1);




    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned int), triangles.data(), GL_STATIC_DRAW);




    glBindVertexArray(0);

    isDirty=true;

    



    }

    void Release()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(5, VBO);
        glDeleteProgram(instanceProgram);
        glDeleteTextures(1, &defaultTextureId);
    }

    void Add(const Vector3 &position)
    {
        translations.push_back(position);
        isDirty=true;
    }

    void Clear()
    {
        translations.clear();
        isDirty=true;
    }

    int Count()
    {
        return (int)translations.size();
    }

    void Render(const float *vp)
    {
        if (isDirty)
        {
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
            glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(Vector3) , translations.data(), GL_DYNAMIC_DRAW);
            glBindVertexArray(0);
            isDirty=false;
        }

        glUseProgram(instanceProgram);
        glUniformMatrix4fv(uniformVP, 1, false, vp);
        glUniform1i(uniformTexture0, 0);

        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, defaultTextureId);
        glBindTexture(GL_TEXTURE_2D_ARRAY, defaultTextureId);

  //x type y start z end w density
        glUniform4f(uniformFogInfos, 2.0, fogStart, fogEnd, fogDensity);
        glUniform3f(uniformFogColor, fogColor.r/255.0f, fogColor.g/255.0f,fogColor.b/255.0f);   
      

        glBindVertexArray(VAO);

        glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0, translations.size());

        
      //  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

        //glDrawArrays(GL_TRIANGLES, 0, 36);
 
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


    }

   

    unsigned int VBO[5];
    unsigned int VAO;
    unsigned int instanceProgram ;
    unsigned int uniformVP;
    unsigned int uniformFogColor;
    unsigned int uniformFogInfos;
    unsigned int uniformTexture0;
    unsigned int defaultTextureId;
    int count;
    std::vector<Vector3> translations;
    bool isDirty;
};

struct Voxel 
{
    Vector3 position;
    Color color;
    float value;
};

InstanceData instances;

float Distance(const Vector3 &a, const Vector3 &b)
{
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrt(dx*dx + dz*dz);
}

struct Chunk
{
    Chunk(const Vector3 &position)
    {
        this->position = position;
        boundingBox.addInternalPoint(position);
        visible = true;
    }
    ~Chunk()
    {
        for (int i = 0; i < (int)voxels.size(); i++)
        {
            delete voxels[i];
        }
        voxels.clear();
    }
    void AddVoxel(const Vector3 &position, const Color &color)
    {
            Voxel *voxel= new Voxel();
            voxel->position = position;
            voxel->color = color;
            voxels.push_back(voxel);
            boundingBox.addInternalPoint(position+Vector3(0.5,0.5,0.5));
            boundingBox.addInternalPoint(position-Vector3(0.5,0.5,0.5));
            
     
    }

//test
    void Build()
    {
        
        for (int i = 0; i < (int)voxels.size(); i++)
        {
            Voxel *voxel = voxels[i];
            instances.Add(voxel->position);
        }
        
      
    }

    void Render(RenderBatch &batch, const Vector3 &camera)
    {
        select = boundingBox.IsPointInBox(camera);
        center = boundingBox.getCenter();
        float distance = Distance(center,camera);
        Color color = Color(255,255,255,255);
        visible = true;
        if (distance >  SIZE*4)
        {
            color = Color(255,0,0,255);
            visible = false;
        }
        if (!frustum.BoundingBoxInView(boundingBox) || !visible)
            return;
     
        for (int i = 0; i < (int)voxels.size(); i++)
        {
            Voxel *voxel = voxels[i];
            instances.Add(voxel->position);
        }
        if (select)
        {
        float height = boundingBox.max.y - boundingBox.min.y;
      //  batch.DrawCube(center, 1,height,1, color);
       // batch.DrawBoundigBox(boundingBox,color);
        }
    }
  
    std::vector<Voxel*> voxels;
    int grid[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    Vector3 center; 
    Vector3 position; 
    BoundingBox boundingBox;
    bool visible;
    bool select;
 
};

float Get2DPerlin (float x, float y, float offset, float scale) 
{
        return noise.GetNoise((x + 0.1f) / SIZE * scale + offset, (y + 0.1f) / SIZE * scale + offset);

 }


 float generateTerrainHeight(float x, float z)
{
    float scaledX = x * 2;
    float scaledZ = z * 2;
    float terrainHeight = noise.GetNoise(scaledX, scaledZ) ;
    terrainHeight *= 20.0f;
    int roundedHeight =10 + static_cast<int>(terrainHeight + 0.5f);
    return roundedHeight;
}


struct World 
{

    void Init()
    {
        
        
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetSeed(seed);







    }
    void Free()
    {
        for (int i = 0; i < (int)chunks.size(); i++)
        {
            delete chunks[i];
        }
      
        chunks.clear();
        vectorSet.clear();

    }

    void Generate(const Vector3 &position)
    {
    int				power;
	float			p;
	float			cave;
	float			height;
	float			width;
	float			biome;

        for (int x = (position.x-SIZE)/CHUNK_SIZE ; x < (position.x + SIZE)/CHUNK_SIZE; x++)
        {
            for (int z = (int)(position.z-SIZE)/CHUNK_SIZE; z < (position.z+ SIZE)/CHUNK_SIZE; z++)
            {
                Vector3 pos(x*CHUNK_SIZE, 0, z*CHUNK_SIZE);
                if (!Contains(pos))
                {
                    Chunk *chunk = new Chunk(pos);
                     for (int i = 0; i < CHUNK_SIZE; i++)
                     {
                        for (int j = 0; j < CHUNK_SIZE; j++)
                        {


              


                                // float n1 = scale1 * noise.GetNoise(0.3f * ((float) (pos.x + i)) + 30310, 0.3f * ((float) (pos.z + j) + 3130));
                                // float n2 = scale2 * noise.GetNoise(2.3f * ((float) (pos.x + i)) + 30310, 2.3f * ((float) (pos.z + j) + 3130));
                                // float n3 = scale3 * noise.GetNoise(5.3f * ((float) (pos.x + i)) + 30310, 5.3f * ((float) (pos.z + j) + 3130));
                                // float terrainHeight= abs(0.4 -  (n1 + n2 + n3 / 3.0f)) * (CHUNK_SIZE/4) * 2.5 ;

            
                            //      float n1 = scale1 * noise.GetNoise(0.3f * ((float) (pos.x + i)) + 30310, 0.3f * ((float) (pos.z + j) + 3130));
                            //      float n2 = scale2 * noise.GetNoise(2.3f * ((float) (pos.x + i)) + 30310, 2.3f * ((float) (pos.z + j) + 3130));
                            //      float n3 = scale3 * noise.GetNoise(5.3f * ((float) (pos.x + i)) + 30310, 5.3f * ((float) (pos.z + j) + 3130));

	                        //        float terrainHeight=-20 +  abs( 0.4 -  (n1 + n2 + n3 / 3.0f)) * (CHUNK_SIZE/4) * 8.5f ;
                            //    //   terrainHeight *= 20.0f;
                            //        int roundedHeight = static_cast<int>(terrainHeight + 0.5f);



                                // Ajusta a altura garantindo que os cubos mantenham uma distância consistente
                                float roundedHeight =generateTerrainHeight((pos.x + i),pos.z + j);//  normalizedNoise * 256;

                           
                                Vector3 voxelPos((x*CHUNK_SIZE)+i, roundedHeight, (z*CHUNK_SIZE)+j);
                                Color color;
                                chunk->AddVoxel(voxelPos, color);
                            
                        }
                     }
                    chunks.push_back(chunk);
                    vectorSet.insert(pos);
                }
            }
        }
    }
    bool Contains(const Vector3 &position)
    {
        return vectorSet.find(position) != vectorSet.end();
    }

    void Build()
    {
        instances.Clear();
        for (int i = 0; i < (int)chunks.size(); i++)
        {
            
            Chunk *chunk = chunks[i];
            chunk->Build();
        }
    }

    void Render(RenderBatch &batch, const Vector3 &camera, const float *vp)
    {
        
        instances.Clear();
        Generate(camera);
        for (int i = 0; i < (int)chunks.size(); i++)
        {
              Chunk *chunk = chunks[i];
              chunk->Render(batch, camera);
        }
        instances.Render(vp);
    }


    int Count()
    {
        //return chunks.size();
        return  instances.Count();
    }   
     std::vector<Chunk*> chunks;
     std::unordered_set<Vector3, Vector3Hash> vectorSet;
     
     Uint32 program;
     Uint32 vao;
    Uint32 vbo;
    

};

float clamp(float x, float lowerlimit, float upperlimit) 
{
    return std::min(upperlimit, std::max(x, lowerlimit));
}

float smoothstep(float edge0, float edge1, float x) 
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

float lerp(float a, float b, float f) 
{
    return a + f * (b - a);
}

int main()
{
 
       if (SDL_Init(SDL_INIT_VIDEO) < 0) 
        {
                Log(2,"SDL could not initialize! Error: %s", SDL_GetError());
            return 1;
        }
        m_shouldclose=false;

    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES)!=0)
    {
        SDL_Log( "ERROR loading context profile mask");
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)!=0)
    {
        SDL_Log("ERROR setting context major version");
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0)!=0)
    {
        SDL_Log("ERROR setting context minor version");
    }
    if(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) != 0){
        SDL_Log( " ERROR \n setting double buffer");
    } // I have tried without the dubble buffer and nothing changes
    if(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE   , 24) != 0){
        SDL_Log( " ERROR \n setting depth buffer");
    }
    if(SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32)!=0){
        SDL_Log( "ERROR setting buffer size");
    }
    if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)!=0){
        SDL_Log( " ERROR loading red");
    }
    if(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8)!=0){
        SDL_Log( " ERROR loading red");
    }if(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)!=0){
        SDL_Log( " ERROR loading red");
    }if(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8)!=0){
        SDL_Log(" Error setting alpha");
    }

   if(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) != 0){
        SDL_Log( " Error  setting stencil buffer");
    }



        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);



        window = SDL_CreateWindow("Batch", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL |SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
        if (!window) 
        {
            Log(2, "Window could not be created! SDL Error: %s" , SDL_GetError() );
            
            return false;
        }

    context = SDL_GL_CreateContext(window);
    if (!context) 
    {
        Log(2, "OpenGL context could not be created! %s ", SDL_GetError());
        SDL_DestroyWindow(window);
        return false; 
    }

        gladLoadGLES2Loader(SDL_GL_GetProcAddress);
    Log(0,"[DEVICE] Vendor  :  %s",glGetString(GL_VENDOR));
    Log(0,"[DEVICE] Renderer:  %s",glGetString(GL_RENDERER));
    Log(0,"[DEVICE] Version :  %s",glGetString(GL_VERSION));

    srand((unsigned int)SDL_GetTicks());              // Initialize random seed


    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    glDisable(GL_DEPTH_TEST);                               // Disable depth testing for 2D (only used for 3D)
    glEnable(GL_MULTISAMPLE);                               // Enable multi sampling (to remove aliasing)

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)

    glCullFace(GL_BACK);                                    // Cull the back face (default)
    glFrontFace(GL_CCW);                                    // Front face are defined counter clockwise (default)
    glEnable(GL_CULL_FACE);                                 // Enable backface culling
    ResizeWindow(SCR_WIDTH,SCR_HEIGHT);
 
    Log(0, "GL: OpenGL device information:");
    Log(0, "    > Vendor:   %s", glGetString(GL_VENDOR));
    Log(0, "    > Renderer: %s", glGetString(GL_RENDERER));
    Log(0, "    > Version:  %s", glGetString(GL_VERSION));
    Log(0, "    > GLSL:     %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    
     SDL_GL_SetSwapInterval(1);


    instances.Init();


    batch.Init(2,1024);
    batch3D.Init(4,2048);

    // SDL_SetRelativeMouseMode(SDL_TRUE);




    World world;
    world.Init();
    WaterPlane water;
    water.Init();
    

    world.Generate(Vector3(0,0,0));
    // world.Generate(Vector3(0,0,32));
    // world.Generate(Vector3(32,0,0));
    // world.Generate(Vector3(32,0,32));
    
    

   


    world.Build();

 
    //instances.Add(Vector3(0,0,0));
    // instances.Add(Vector3(1,0,0));
    // instances.Add(Vector3(0,0,1));
    // instances.Add(Vector3(0,0,-1));



        SkyBox skybox;
        skybox.Init();
    

    double lastTime = GetTime();
    float fogWaterAdd=1.0f;
    float fogWaterSub=1.0f;
    int frameCount = 0;
    double fps = 0.0;
    bool onWater = false;
    bool fogAnimated = false;


    while(Run())
    {

        float deltaTime = GetTime() - lastTime;

        const Uint8 *state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_W])
            camera.ProcessKeyboard(FORWARD, 0.1);   
        if (state[SDL_SCANCODE_S])
            camera.ProcessKeyboard(BACKWARD, 0.1);
        if (state[SDL_SCANCODE_A])
            camera.ProcessKeyboard(LEFT, 0.1);
        if (state[SDL_SCANCODE_D])
            camera.ProcessKeyboard(RIGHT, 0.1);

        // if (IsMouseDown(SDL_BUTTON_LEFT))
        // {
        //     float x = rand() % 100;
        //     float y = rand() % 100;
        //     float z = rand() % 100;
        //     translations.push_back(Vector3( x,y,z));
        // }

        
        

        float xpos = mousePosition.x;
        float ypos = mousePosition.y;
        if (firstMouse)
        {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);

 
 

        glClearColor(fogColor.r/255.0f, fogColor.g/255.0f,fogColor.b/255.0f, 1.0f);   
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 


        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, MAX_DISTANCE);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = projection * view * model;  
        batch3D.setProjection(glm::value_ptr(mvp));
        frustum.CalculateFrustum(glm::value_ptr(view), glm::value_ptr(projection));



     //   camera.Position.y = 3.0f;

        Vector3 position(camera.Position.x, 0, camera.Position.z);

        if (camera.Position.y <= 1.20)
        {   
               onWater = true;
                fogColor = fogWater;
              //  fogDensity = 0.1;
                fogStart = 100.0;
         } else 
        {
            onWater = false;
            fogColor = fogDefaul;
           // fogDensity = 0.005;
              fogStart = SIZE*2;


        }
            
            if (onWater)
            {
               
                    fogWaterSub=0.1f;
                    if (fogWaterAdd<1.0f)
                    {
                        fogWaterAdd += deltaTime  * 0.05f;
                        fogDensity = lerp(fogDensity,0.1,fogWaterAdd);
                    } else
                    {
                        fogDensity = 0.1;
                    } 
     
                    skybox.underwater=true;
                    skybox.colorForce=1.0f;
           
                       
                

            } else 
            {
                  fogWaterAdd=0.0f;
                  skybox.underwater=false;

                
              

                if (fogWaterSub<1.0f)
                {
                    fogWaterSub += deltaTime  * 0.005f;
                    fogDensity = lerp(fogDensity,0.005,fogWaterSub);
         
                } else 
                {
                    fogDensity = 0.005;
           
         
                }
               

            }


 



        //draw instance
        glm::mat4 vp = projection * view;
       



     //   world.Render(batch3D, position, glm::value_ptr(vp));
       // instances.Render(glm::value_ptr(vp));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

       
        float gridSize = (SIZE*2) * 0.5f;
        int cellX = (int)(position.x / gridSize);
        int cellZ = (int)(position.z / gridSize);
        Vector3 waterPlanePosition = Vector3(cellX * gridSize + gridSize / 2.0f, 0, cellZ * gridSize + gridSize / 2.0f);
      //  skybox.Render(projection, view);
        world.Render(batch3D, position, glm::value_ptr(vp));



        water.Render(waterPlanePosition, projection, view);
        
        
        skybox.Render(projection, view, deltaTime);

         
      

     //    batch3D.DrawCube(Vector3(0,0,0),1,1,1,Color(255,0,0,255));
        // batch3D.DrawCubeLines(Vector3(2,0,0),1,1,1,Color(0,255,0,255));

        glDisable(GL_CULL_FACE);


        
        batch3D.DrawGrid(10,1);


        batch3D.Render();


        

        batch.DrawCircle(100,100,5,Color(255,0,0,255));

        glDisable(GL_DEPTH_TEST);        
        batch.Render();

        

        double currentTime = GetTime();
        frameCount++;

        if (currentTime - lastTime >= 1.0)
         {
            fps = frameCount / (currentTime - lastTime);

            
        
            SDL_SetWindowTitle(window,TextFormat("FPS: %i %d (%f)",int(fps),(int)world.Count(),deltaTime));

            frameCount = 0;
            lastTime = currentTime;
        }


        Swap();

            

    }   

    skybox.Free();

    water.Free();
    world.Free();
    instances.Release();
    texture.Release();
    
    batch.Release();
    Log(0,"[DEVICE] Close and terminate .");
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}
