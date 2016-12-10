/*Header file
 *Contains Macro Definitions
 *also contains Structure Definitions 
 *These are to be used by multiple files.
 */
#define GLFW_DLL 1
#define GL_GLEXT_PROTOTYPES
#define M_PI acos(-1.0)

typedef struct{
  float Position[2];
  float TexCoord[2];
}vertex;

typedef struct{
  float scale;
  float rotate;
  float translate[2];
  float shear[2];
}affine;
