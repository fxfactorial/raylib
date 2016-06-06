/**********************************************************************************************
*
*   rlgl - raylib OpenGL abstraction layer
*
*   raylib now uses OpenGL 1.1 style functions (rlVertex) that are mapped to selected OpenGL version:
*       OpenGL 1.1  - Direct map rl* -> gl*
*       OpenGL 3.3  - Vertex data is stored in VAOs, call rlglDraw() to render
*       OpenGL ES 2 - Vertex data is stored in VBOs or VAOs (when available), call rlglDraw() to render
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RLGL_H
#define RLGL_H

//#define RLGL_STANDALONE       // NOTE: To use rlgl as standalone lib, just uncomment this line

#ifndef RLGL_STANDALONE
    #include "raylib.h"         // Required for: Model, Shader, Texture2D
    #include "utils.h"          // Required for: TraceLog()
#endif

#ifdef RLGL_STANDALONE
    #define RAYMATH_STANDALONE
#endif

#include "raymath.h"            // Required for: Vector3, Matrix

// Select desired OpenGL version
// NOTE: Those preprocessor defines are only used on rlgl module,
// if OpenGL version is required by any other module, it uses rlGetVersion()

// Choose opengl version here or just define it at compile time: -DGRAPHICS_API_OPENGL_33
//#define GRAPHICS_API_OPENGL_11     // Only available on PLATFORM_DESKTOP
//#define GRAPHICS_API_OPENGL_33     // Only available on PLATFORM_DESKTOP
//#define GRAPHICS_API_OPENGL_ES2    // Only available on PLATFORM_ANDROID or PLATFORM_RPI or PLATFORM_WEB

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && !defined(GRAPHICS_API_OPENGL_33) && !defined(GRAPHICS_API_OPENGL_ES2)
    #define GRAPHICS_API_OPENGL_11
#endif

// Security check in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(GRAPHICS_API_OPENGL_33)
        #undef GRAPHICS_API_OPENGL_33
    #endif

    #if defined(GRAPHICS_API_OPENGL_ES2)
        #undef GRAPHICS_API_OPENGL_ES2
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: This is the maximum amount of lines, triangles and quads per frame, be careful!
    #define MAX_LINES_BATCH         8192
    #define MAX_TRIANGLES_BATCH     4096
    #define MAX_QUADS_BATCH         4096
#elif defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: Reduce memory sizes for embedded systems (RPI and HTML5)
    // NOTE: On HTML5 (emscripten) this is allocated on heap, by default it's only 16MB!...just take care...
    #define MAX_LINES_BATCH         1024    // Critical for wire shapes (sphere)
    #define MAX_TRIANGLES_BATCH     2048    // Critical for some shapes (sphere)
    #define MAX_QUADS_BATCH         1024    // Be careful with text, every letter maps a quad
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { RL_PROJECTION, RL_MODELVIEW, RL_TEXTURE } MatrixMode;

typedef enum { RL_LINES, RL_TRIANGLES, RL_QUADS } DrawMode;

typedef enum { OPENGL_11 = 1, OPENGL_33, OPENGL_ES_20 } GlVersion;

#if defined(RLGL_STANDALONE)
    #ifndef __cplusplus
    // Boolean type
    typedef enum { false, true } bool;
    #endif

    // byte type
    typedef unsigned char byte;
    
    // Color type, RGBA (32bit)
    typedef struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } Color;

    // Texture formats (support depends on OpenGL version)
    typedef enum { 
        UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
        UNCOMPRESSED_GRAY_ALPHA,
        UNCOMPRESSED_R5G6B5,            // 16 bpp
        UNCOMPRESSED_R8G8B8,            // 24 bpp
        UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
        UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
        UNCOMPRESSED_R8G8B8A8,          // 32 bpp
        COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
        COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
        COMPRESSED_DXT3_RGBA,           // 8 bpp
        COMPRESSED_DXT5_RGBA,           // 8 bpp
        COMPRESSED_ETC1_RGB,            // 4 bpp
        COMPRESSED_ETC2_RGB,            // 4 bpp
        COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
        COMPRESSED_PVRT_RGB,            // 4 bpp
        COMPRESSED_PVRT_RGBA,           // 4 bpp
        COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
        COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
    } TextureFormat;

    // Vertex data definning a mesh
    typedef struct Mesh {
        int vertexCount;        // number of vertices stored in arrays
        int triangleCount;      // number of triangles stored (indexed or not)
        float *vertices;        // vertex position (XYZ - 3 components per vertex) (shader-location = 0)
        float *texcoords;       // vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
        float *texcoords2;      // vertex second texture coordinates (useful for lightmaps) (shader-location = 5)
        float *normals;         // vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
        float *tangents;        // vertex tangents (XYZ - 3 components per vertex) (shader-location = 4)
        unsigned char *colors;  // vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
        unsigned short *indices;// vertex indices (in case vertex data comes indexed)

        unsigned int vaoId;     // OpenGL Vertex Array Object id
        unsigned int vboId[7];  // OpenGL Vertex Buffer Objects id (7 types of vertex data)
    } Mesh;

    // Shader type (generic shader)
    typedef struct Shader {
        unsigned int id;        // Shader program id
        
        // Vertex attributes locations (default locations)
        int vertexLoc;          // Vertex attribute location point (default-location = 0)
        int texcoordLoc;        // Texcoord attribute location point (default-location = 1)
        int normalLoc;          // Normal attribute location point (default-location = 2)
        int colorLoc;           // Color attibute location point (default-location = 3)
        int tangentLoc;         // Tangent attribute location point (default-location = 4)
        int texcoord2Loc;       // Texcoord2 attribute location point (default-location = 5)

        // Uniform locations
        int mvpLoc;             // ModelView-Projection matrix uniform location point (vertex shader)
        int tintColorLoc;       // Color uniform location point (fragment shader)
        
        // Texture map locations (generic for any kind of map)
        int mapTexture0Loc;     // Map texture uniform location point (default-texture-unit = 0)
        int mapTexture1Loc;     // Map texture uniform location point (default-texture-unit = 1)
        int mapTexture2Loc;     // Map texture uniform location point (default-texture-unit = 2)
    } Shader;

    // Texture2D type
    // NOTE: Data stored in GPU memory
    typedef struct Texture2D {
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (TextureFormat)
    } Texture2D;
    
    // RenderTexture2D type, for texture rendering
    typedef struct RenderTexture2D {
        unsigned int id;        // Render texture (fbo) id
        Texture2D texture;      // Color buffer attachment texture
        Texture2D depth;        // Depth buffer attachment texture
    } RenderTexture2D;
    
    // Material type
    typedef struct Material {
        Shader shader;          // Standard shader (supports 3 map types: diffuse, normal, specular)

        Texture2D texDiffuse;   // Diffuse texture
        Texture2D texNormal;    // Normal texture
        Texture2D texSpecular;  // Specular texture

        Color colDiffuse;       // Diffuse color
        Color colAmbient;       // Ambient color
        Color colSpecular;      // Specular color
        
        float glossiness;       // Glossiness level (Ranges from 0 to 1000)
    } Material;
    
    // Camera type, defines a camera position/orientation in 3d space
    typedef struct Camera {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view apperture in Y (degrees)
    } Camera;

    // Light type
    typedef struct LightData {
        unsigned int id;        // Light unique id
        int type;               // Light type: LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT
        bool enabled;           // Light enabled
        
        Vector3 position;       // Light position
        Vector3 target;         // Light target: LIGHT_DIRECTIONAL and LIGHT_SPOT (cone direction target)
        float radius;           // Light attenuation radius light intensity reduced with distance (world distance)
        
        Color diffuse;          // Light diffuse color
        float intensity;        // Light intensity level
        
        float coneAngle;        // Light cone max angle: LIGHT_SPOT
    } LightData, *Light;
    
    // Light types
    typedef enum { LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT } LightType;

    // Color blending modes (pre-defined)
    typedef enum { BLEND_ALPHA = 0, BLEND_ADDITIVE, BLEND_MULTIPLIED } BlendMode;
#endif

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------
void rlMatrixMode(int mode);                    // Choose the current matrix to be transformed
void rlPushMatrix(void);                        // Push the current matrix to stack
void rlPopMatrix(void);                         // Pop lattest inserted matrix from stack
void rlLoadIdentity(void);                      // Reset current matrix to identity matrix
void rlTranslatef(float x, float y, float z);   // Multiply the current matrix by a translation matrix
void rlRotatef(float angleDeg, float x, float y, float z);  // Multiply the current matrix by a rotation matrix
void rlScalef(float x, float y, float z);       // Multiply the current matrix by a scaling matrix
void rlMultMatrixf(float *mat);                 // Multiply the current matrix by another matrix
void rlFrustum(double left, double right, double bottom, double top, double near, double far);
void rlOrtho(double left, double right, double bottom, double top, double near, double far);
void rlViewport(int x, int y, int width, int height); // Set the viewport area

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
void rlBegin(int mode);                         // Initialize drawing mode (how to organize vertex)
void rlEnd(void);                               // Finish vertex providing
void rlVertex2i(int x, int y);                  // Define one vertex (position) - 2 int
void rlVertex2f(float x, float y);              // Define one vertex (position) - 2 float
void rlVertex3f(float x, float y, float z);     // Define one vertex (position) - 3 float
void rlTexCoord2f(float x, float y);            // Define one vertex (texture coordinate) - 2 float
void rlNormal3f(float x, float y, float z);     // Define one vertex (normal) - 3 float
void rlColor4ub(byte r, byte g, byte b, byte a);    // Define one vertex (color) - 4 byte
void rlColor3f(float x, float y, float z);          // Define one vertex (color) - 3 float
void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL equivalent functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer
//------------------------------------------------------------------------------------
void rlEnableTexture(unsigned int id);          // Enable texture usage
void rlDisableTexture(void);                    // Disable texture usage
void rlEnableRenderTexture(unsigned int id);    // Enable render texture (fbo)
void rlDisableRenderTexture(void);              // Disable render texture (fbo), return to default framebuffer
void rlEnableDepthTest(void);                   // Enable depth test
void rlDisableDepthTest(void);                  // Disable depth test
void rlEnableWireMode(void);                    // Enable wire mode
void rlDisableWireMode(void);                   // Disable wire mode
void rlDeleteTextures(unsigned int id);         // Delete OpenGL texture from GPU
void rlDeleteRenderTextures(RenderTexture2D target);    // Delete render textures (fbo) from GPU
void rlDeleteShader(unsigned int id);           // Delete OpenGL shader program from GPU
void rlDeleteVertexArrays(unsigned int id);     // Unload vertex data (VAO) from GPU memory
void rlDeleteBuffers(unsigned int id);          // Unload vertex data (VBO) from GPU memory
void rlClearColor(byte r, byte g, byte b, byte a);  // Clear color buffer with color
void rlClearScreenBuffers(void);                // Clear used screen buffers (color and depth)
int rlGetVersion(void);                         // Returns current OpenGL version

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
void rlglInit(void);                            // Initialize rlgl (shaders, VAO, VBO...)
void rlglClose(void);                           // De-init rlgl
void rlglDraw(void);                            // Draw VAO/VBO
void rlglInitGraphics(int offsetX, int offsetY, int width, int height);  // Initialize Graphics (OpenGL stuff)

unsigned int rlglLoadTexture(void *data, int width, int height, int textureFormat, int mipmapCount);    // Load texture in GPU
RenderTexture2D rlglLoadRenderTexture(int width, int height);   // Load a texture to be used for rendering (fbo with color and depth attachments)
void rlglUpdateTexture(unsigned int id, int width, int height, int format, void *data);         // Update GPU texture with new data
void rlglGenerateMipmaps(Texture2D texture);                             // Generate mipmap data for selected texture

void rlglLoadMesh(Mesh *mesh, bool dynamic);                        // Upload vertex data into GPU and provided VAO/VBO ids
void rlglUpdateMesh(Mesh mesh, int buffer, int numVertex);          // Update vertex data on GPU (upload new data to one buffer)
void rlglDrawMesh(Mesh mesh, Material material, Matrix transform);  // Draw a 3d mesh with material and transform
void rlglUnloadMesh(Mesh *mesh);                                    // Unload mesh data from CPU and GPU

Vector3 rlglUnproject(Vector3 source, Matrix proj, Matrix view);    // Get world coordinates from screen coordinates

unsigned char *rlglReadScreenPixels(int width, int height);         // Read screen pixel data (color buffer)
void *rlglReadTexturePixels(Texture2D texture);                     // Read texture pixel data

// NOTE: There is a set of shader related functions that are available to end user,
// to avoid creating function wrappers through core module, they have been directly declared in raylib.h

#if defined(RLGL_STANDALONE)
//------------------------------------------------------------------------------------
// Shaders System Functions (Module: rlgl)
// NOTE: This functions are useless when using OpenGL 1.1
//------------------------------------------------------------------------------------
Shader LoadShader(char *vsFileName, char *fsFileName);              // Load a custom shader and bind default locations
void UnloadShader(Shader shader);                                   // Unload a custom shader from memory

Shader GetDefaultShader(void);                                      // Get default shader
Shader GetStandardShader(void);                                     // Get default shader
Texture2D GetDefaultTexture(void);                                  // Get default texture

int GetShaderLocation(Shader shader, const char *uniformName);              // Get shader uniform location
void SetShaderValue(Shader shader, int uniformLoc, float *value, int size); // Set shader uniform value (float)
void SetShaderValuei(Shader shader, int uniformLoc, int *value, int size);  // Set shader uniform value (int)
void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat);       // Set shader uniform value (matrix 4x4)

void SetMatrixProjection(Matrix proj);                              // Set a custom projection matrix (replaces internal projection matrix)
void SetMatrixModelview(Matrix view);                               // Set a custom modelview matrix (replaces internal modelview matrix)

void BeginShaderMode(Shader shader);                                // Begin custom shader drawing
void EndShaderMode(void);                                           // End custom shader drawing (use default shader)
void BeginBlendMode(int mode);                                      // Begin blending mode (alpha, additive, multiplied)
void EndBlendMode(void);                                            // End blending mode (reset to default: alpha blending)

Light CreateLight(int type, Vector3 position, Color diffuse);       // Create a new light, initialize it and add to pool
void DestroyLight(Light light);                                     // Destroy a light and take it out of the list
#endif

#ifdef __cplusplus
}
#endif

#endif // RLGL_H