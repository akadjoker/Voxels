#include <iostream>
#include "Batch.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "FastNoise.h"

#define FNL_IMPL
#include "fastnoiselite.h"



unsigned int SCR_WIDTH = 1024;
unsigned int SCR_HEIGHT = 720;

#define MAX_BATCH_ELEMENTS  8192

Color fogDefaul(0.2*255,0.2*255,0.2*255);
Color fogWater(0.5*255,0.7*255,1.0*255);

float fogStart = 200.0;
float fogEnd = 1000.0;
float fogDensity = 0.01;

Color fogColor(0.2*255,0.2*255,0.2*255);

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


const int CHUNK_SIZE = 16;
//5 chunks 
const int SIZE = 128;//8 * CHUNK_SIZE;

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

    float x_tiles =4.0f;
    float y_tiles =4.0f;
    float texture_width = 1024.0f;  // Largura da textura original
    float texture_height = 1024.0f;  // Altura da textura original

    float W =SIZE  / x_tiles;  // Largura de cada tile em coordenadas de textura
    float H =SIZE  / y_tiles;  // Altura de cada tile em coordenadas de textura


 float vertices[] = {
        // positions          // colors           // texture coords
         SIZE,  1.1f, SIZE,   1.0f, 1.0f, 1.0f,0.4f,   W, H, // top right
         SIZE,  1.1f, -SIZE,   1.0f, 1.0f, 1.0f,0.4f,   W, 0.0f, // bottom right
        -SIZE,  1.1f, -SIZE,   1.0f, 1.0f, 1.0f,0.4f,   0.0f, 0.0f, // bottom left
        -SIZE,  1.1f, SIZE,   1.0f, 1.0f, 1.0f,0.4f,   0.0f, H  // top left 
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
            in vec3 position;            
            out vec2 fragTexCoord;             
            out flat int textureIndex;   

                 
            uniform mat4 vp;                  
            void main()                        
            {                                  
                fragTexCoord = uv; 
                gl_Position = vp * vec4(pos + position, 1.0); 
                textureIndex = 1;

                // if  ( position.y  > 12.0)
                // {
                //     textureIndex = 8;
                // } else 
                // {
                //     if (position.y <= 1.0)
                //         textureIndex = 3;
                //     else if (position.y >= 2.0 && position.y <= 4.0)
                //         textureIndex = 0;
                    

                // }

              
        if (position.y <= 0.0) 
        {
            textureIndex = 4; // Água
        } else if (position.y <= 1.0) {
            textureIndex = 3; // Terra
        } else if (position.y <= 12.0) {
            textureIndex = 1; 
        } else 
        {
            textureIndex = 0; // Acima da altura 12, use textura padrão
        }  



            };)";


            const char *defaultFShaderCode =R"(
            #version 320 es      

            #define FOGMODE_EXP 1.
            #define FOGMODE_EXP2 2.
            #define FOGMODE_LINEAR 3.
            #define E 2.71828



            precision mediump sampler2DArray;
            precision mediump float;           


            

            in vec2 fragTexCoord;              
            in flat int  textureIndex;
            out vec4 finalColor;               
            uniform sampler2DArray texture0;    


            uniform vec4 uFogInfos;
            uniform vec3 uFogColor;


            float calcFogFactor() 
            {
                // gets distance from camera to vertex
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

float cubeVertices[] = 
{
    // Face frontal
   -0.5f, -0.5f,  0.5f,  // V0
    0.5f, -0.5f,  0.5f,  // V1
    0.5f,  0.5f,  0.5f,  // V2
    0.5f,  0.5f,  0.5f,  // V3
   -0.5f,  0.5f,  0.5f,  // V4
   -0.5f, -0.5f,  0.5f,  // V5

    // Face traseira
   -0.5f, -0.5f, -0.5f,  // V6
    0.5f, -0.5f, -0.5f,  // V7
    0.5f,  0.5f, -0.5f,  // V8
    0.5f,  0.5f, -0.5f,  // V9
   -0.5f,  0.5f, -0.5f,  // V10
   -0.5f, -0.5f, -0.5f,  // V11

    // Face superior
   -0.5f,  0.5f,  0.5f,  // V12
    0.5f,  0.5f,  0.5f,  // V13
    0.5f,  0.5f, -0.5f,  // V14
    0.5f,  0.5f, -0.5f,  // V15
   -0.5f,  0.5f, -0.5f,  // V16
   -0.5f,  0.5f,  0.5f,  // V17

    // Face inferior
   -0.5f, -0.5f,  0.5f,  // V18
    0.5f, -0.5f,  0.5f,  // V19
    0.5f, -0.5f, -0.5f,  // V20
    0.5f, -0.5f, -0.5f,  // V21
   -0.5f, -0.5f, -0.5f,  // V22
   -0.5f, -0.5f,  0.5f,  // V23

    // Face direita
    0.5f, -0.5f,  0.5f,  // V24
    0.5f, -0.5f, -0.5f,  // V25
    0.5f,  0.5f, -0.5f,  // V26
    0.5f,  0.5f, -0.5f,  // V27
    0.5f,  0.5f,  0.5f,  // V28
    0.5f, -0.5f,  0.5f,  // V29

    // Face esquerda
   -0.5f, -0.5f,  0.5f,  // V30
   -0.5f, -0.5f, -0.5f,  // V31
   -0.5f,  0.5f, -0.5f,  // V32
   -0.5f,  0.5f, -0.5f,  // V33
   -0.5f,  0.5f,  0.5f,  // V34
   -0.5f, -0.5f,  0.5f   // V35
};

float cubeUVs[] = 
{
    0.0f, 0.0f,  // V0
    1.0f, 0.0f,  // V1
    1.0f, 1.0f,  // V2
    1.0f, 1.0f,  // V3
    0.0f, 1.0f,  // V4
    0.0f, 0.0f,  // V5

    0.0f, 0.0f,  // V6
    1.0f, 0.0f,  // V7
    1.0f, 1.0f,  // V8
    1.0f, 1.0f,  // V9
    0.0f, 1.0f,  // V10
    0.0f, 0.0f,  // V11

    0.0f, 0.0f,  // V12
    1.0f, 0.0f,  // V13
    1.0f, 1.0f,  // V14
    1.0f, 1.0f,  // V15
    0.0f, 1.0f,  // V16
    0.0f, 0.0f,  // V17

    0.0f, 0.0f,  // V18
    1.0f, 0.0f,  // V19
    1.0f, 1.0f,  // V20
    1.0f, 1.0f,  // V21
    0.0f, 1.0f,  // V22
    0.0f, 0.0f,  // V23

    0.0f, 0.0f,  // V24
    1.0f, 0.0f,  // V25
    1.0f, 1.0f,  // V26
    1.0f, 1.0f,  // V27
    0.0f, 1.0f,  // V28
    0.0f, 0.0f,  // V29

    0.0f, 0.0f,  // V30
    1.0f, 0.0f,  // V31
    1.0f, 1.0f,  // V32
    1.0f, 1.0f,  // V33
    0.0f, 1.0f,  // V34
    0.0f, 0.0f   // V35
};




    uniformVP       = glGetUniformLocation(instanceProgram, "vp");
    uniformTexture0 = glGetUniformLocation(instanceProgram, "texture0");
    uniformFogColor = glGetUniformLocation(instanceProgram, "uFogColor");
    uniformFogInfos = glGetUniformLocation(instanceProgram, "uFogInfos");


    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glGenBuffers(3, VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);



    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeUVs), cubeUVs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);



    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(Vector3) , translations.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);
    glVertexAttribDivisor(2, 1);



    glBindVertexArray(0);

    isDirty=true;

    



    }

    void Free()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(3, VBO);
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
            glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
            glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(Vector3) , translations.data(), GL_DYNAMIC_DRAW);
            glBindVertexArray(0);
            isDirty=false;
        }

        glUseProgram(instanceProgram);
        glUniformMatrix4fv(uniformVP, 1, false, vp);
        glUniform1i(uniformTexture0, 0);

//x type y start z end w density
        glUniform4f(uniformFogInfos, 2.0, fogStart, fogEnd, fogDensity);
        glUniform3f(uniformFogColor, fogColor.r/255.0f, fogColor.g/255.0f,fogColor.b/255.0f);   
        
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, defaultTextureId);
        glBindTexture(GL_TEXTURE_2D_ARRAY, defaultTextureId);


        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, translations.size());

        //glDrawArrays(GL_TRIANGLES, 0, 36);
 
        glBindVertexArray(0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


    }

   

    unsigned int VBO[3];
    unsigned int VAO;
    unsigned int instanceProgram ;
    unsigned int uniformVP;
    unsigned int uniformFogColor;
    unsigned int uniformFogInfos;
    unsigned int uniformTexture0;
    unsigned int defaultTextureId;
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

struct Chunk
{
    Chunk(const Vector3 &position)
    {
        center = position;
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
            boundingBox.addInternalPoint(position);
     
    }

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
        //  float dist = center.Distance(camera);
        //  if (dist > 1000)
        //      return;

        if (!frustum.BoundingBoxInView(boundingBox))
            return;
     
        for (int i = 0; i < (int)voxels.size(); i++)
        {
            Voxel *voxel = voxels[i];
           // batch.DrawCube(voxel->position, 1, 1, 1, voxel->color);
            instances.Add(voxel->position);
        }
      //  batch.DrawBoundigBox(boundingBox, Color(255,255,255,255));
    }

    std::vector<Voxel*> voxels;
    Vector3 center;  
    BoundingBox boundingBox;

};

struct World 
{

    void Init()
    {
          srand(time(0));
	    int seed = rand() % __INT_MAX__;
         
        // noise.SetNoiseType(FastNoise::SimplexFractal);
        // noise.SetFrequency(0.01);
        // noise.SetFractalOctaves(4);
        // noise.SetFractalLacunarity(2.0);
        // noise.SetFractalGain(0.5);
        // noise.SetSeed(seed);
    

    	noises = fnlCreateState();
	    noises.noise_type = FNL_NOISE_PERLIN;
	    noises.seed = seed;
	    noises.fractal_type = FNL_FRACTAL_RIDGED;
        








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


                    float noise3d = fnlGetNoise2D(&noises, i + pos.x, j + pos.z);
			        float value = round(CHUNK_SIZE * noise3d);

                             //   float value = noise.GetNoise(i + pos.x, j + pos.z)  * CHUNK_SIZE;
                            // float rawValue = noise.GetNoise(i + pos.x, j + pos.z) * CHUNK_SIZE;
                            // int value = static_cast<int>(std::round(rawValue));

                        

                        //     float value = noise.GetNoise(i + pos.x, j + pos.z) * CHUNK_SIZE;
                            // int value = static_cast<int>(std::round(rawValue));


                        // float scaleFactor = 1.0 ;
                        // float scaledX = (i + pos.x) * scaleFactor;
                        // float scaledZ = (j + pos.z) * scaleFactor;

                        // float rawValue = noise.GetNoise(scaledX, scaledZ) * CHUNK_SIZE;
                        // int value = static_cast<int>(std::round(rawValue)) ;

                       
                              




                                 //float value = noise.GetNoise( i + (x*CHUNK_SIZE), j + (z*CHUNK_SIZE)) ; 
                                // float minRawValue = 0;
                                // float maxRawValue = 1;

                                // // //float normalizedValue = 2.0f * ((value - minRawValue) / (maxRawValue - minRawValue)) - 1.0f;
                                // float normalizedValue = (value- minRawValue) / (maxRawValue - minRawValue);

                                // value = normalizedValue * CHUNK_SIZE;
                             //  float value =perlinNoiseGenerator.generateHeight(i + (x*CHUNK_SIZE), j + (z*CHUNK_SIZE));

                           
                                Vector3 voxelPos((x*CHUNK_SIZE)+i, value, (z*CHUNK_SIZE)+j);
                                Color color;
                                color.r = rand() % 255;
                                color.g = rand() % 255;
                                color.b = rand() % 255;
                                color.a = 255;

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
     FastNoise noise;
     Uint32 program;
     Uint32 vao;
    Uint32 vbo;
    fnl_state noises;

};



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


    batch.Init(8,MAX_BATCH_ELEMENTS);
    batch3D.Init(8,MAX_BATCH_ELEMENTS);

    // SDL_SetRelativeMouseMode(SDL_TRUE);



    WaterPlane water;
    water.Init();

    World world;
    world.Init();
    

   // world.Generate(Vector3(0,0,0));
    //world.Build();

 
    //instances.Add(Vector3(0,0,0));
    // instances.Add(Vector3(1,0,0));
    // instances.Add(Vector3(0,0,1));
    // instances.Add(Vector3(0,0,-1));




    

    double lastTime = GetTime();
    int frameCount = 0;
    double fps = 0.0;

    while(Run())
    {

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
        glDisable(GL_CULL_FACE);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = projection * view * model;  
        batch3D.setProjection(glm::value_ptr(mvp));
        frustum.CalculateFrustum(glm::value_ptr(view), glm::value_ptr(projection));



     //   camera.Position.y = 3.0f;

        Vector3 position(camera.Position.x, 0, camera.Position.z);

        if (camera.Position.y <= 1.20)
        {   
                fogColor = fogWater;
                fogDensity = 0.1;
                fogStart = 10.0;
        } else 
        {
            fogColor = fogDefaul;
            fogDensity = 0.02;
            fogStart = 200.0;
        }
            


 



        //draw instance
        glm::mat4 vp = projection * view;
       



        world.Render(batch3D, position, glm::value_ptr(vp));
         instances.Render(glm::value_ptr(vp));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         water.Render(position, projection, view);

         
       // world.Render(batch3D, position, glm::value_ptr(vp));


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

            
        
            SDL_SetWindowTitle(window,TextFormat("FPS: %i %d (%f)",int(fps),(int)world.Count(),camera.Position.y));

            frameCount = 0;
            lastTime = currentTime;
        }


        Swap();

            

    }   

    water.Free();
    world.Free();
    instances.Free();
    texture.Release();
    
    batch.Release();
    Log(0,"[DEVICE] Close and terminate .");
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}
