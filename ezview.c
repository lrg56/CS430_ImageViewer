#include "ezview.h"
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//Global variables
int width, height, maxColor, version;
GLFWwindow *window;
GLuint vBuffer, vShader, fShader, program;
GLint mvpLocation, vPosLocation, vColLocation;
pixel *buffer;
affine *mat;
vertex vertexes[]={
  {{1,-1}, {0.99999,0.99999}},
  {{1,1}, {0.99999,0.00000}},
  {{-1,1}, {0.00000,0.00000}},
  {{-1,1}, {0.00000,0.00000}},
  {{-1,-1}, {0.00000,0.99999}},
  {{1,-1}, {0.99999,0.99999}}
};
//Compiles the shader
static inline void compileShader(GLuint shader){
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled){
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("ERROR: Shader did not compile.\n");
    exit(1);
  }
}

static void error_callback(int error, const char* description){
     fprintf(stderr, "Error: %s\n", description);
}
//takes the key pressed and does the appropriate action
static void keyCallBack(GLFWwindow* window, int key,int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
      mat[0].scale *= 1.1;
    
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    	mat[0].scale /= 1.1;
    
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    	mat[0].translate[0] += 0.1;
    
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    	mat[0].translate[0] -= 0.1;
    
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
      mat[0].translate[1] += 0.1;
    
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
      mat[0].translate[1] -= 0.1;
    
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    	mat[0].shear[1] += 0.1;
    
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    	mat[0].shear[1] -= 0.1;
    
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    	mat[0].shear[0] += 0.1;
    
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    	mat[0].shear[0] -= 0.1;
}
//intializes all the settings for GLFW and GLES2
//init to win it
int init(){
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()){
    fprintf(stderr, "Could not initialize GLFW.\n");
    exit(1);
  }

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(800, 600, "Image Viewer", NULL, NULL);
  if(!window){
    fprintf(stderr, "ERROR: Window could not open.\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
}

  glfwSetKeyCallback(window, keyCallBack);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glGenBuffers(1, &vBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

  vShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShader, 1, &vertex_shader_text, NULL);
  compileShader(vShader);

  fShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fShader, 1, &fragment_shader_text, NULL);
  compileShader(fShader);

  program = glCreateProgram();
  glAttachShader(program, vShader);
  glAttachShader(program, fShader);
  glLinkProgram(program);

  mvpLocation = glGetUniformLocation(program, "MVP");
  assert(mvpLocation != -1);

  vPosLocation = glGetAttribLocation(program, "vPos");
  assert(vPosLocation != -1);

  GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
  assert(texcoord_location != -1);

  GLint tex_location = glGetUniformLocation(program, "Texture");
  assert(tex_location != -1);

  glEnableVertexAttribArray(vPosLocation);
  glVertexAttribPointer(vPosLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),(void*) 0);
  glEnableVertexAttribArray(texcoord_location);
  glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) (sizeof(float) * 2));
  GLuint texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);
  glUniform1i(tex_location, 0);
  return 1;
}
//runs after the init initialization function
void run() {

  mat = (affine *) malloc(sizeof(affine));
  mat[0].translate[0] = 0.0;
  mat[0].translate[1] = 0.0;
  mat[0].shear[0]     = 0.0;
  mat[0].shear[1]     = 0.0;
  mat[0].rotate       = 0.0;
  mat[0].scale        = 1.0;

  while (!glfwWindowShouldClose(window)) {
    float ratio;
    int imgWidth, imgHeight;
    mat4x4 rotate, shear, scale, translate, rs, rss, affMat;
    glfwGetFramebufferSize(window, &imgWidth, &imgHeight);
    ratio = imgWidth / (float) imgHeight;

    glViewport(0, 0, imgWidth, imgHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    mat4x4_identity(rotate);
    mat4x4_rotate_Z(rotate, rotate, mat[0].rotate);

    mat4x4_identity(shear);
    shear[1][0] = mat[0].shear[0];
    shear[0][1] = mat[0].shear[1];

    mat4x4_identity(scale);
    scale[0][0] = scale[0][0] * mat[0].scale;
    scale[1][1] = scale[1][1] * mat[0].scale;

    mat4x4_identity(translate);
    mat4x4_translate(translate, mat[0].translate[0], mat[0].translate[1], 0);

    mat4x4_mul(rs, rotate, shear);
    mat4x4_mul(rss, rs, scale);
    mat4x4_mul(affMat, rss, translate);

    glUseProgram(program);
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat*) affMat);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(0);
}

//main function for running program
int main(int argc, char *argv[]){

  if (argc != 2){
    fprintf(stderr, "ERROR: Incorrect number of arguments recieved expected 2, found %d\n", argc);
    exit(1);
  }

  FILE *inFile;
  inFile = fopen(argv[1], "rb");
  int error = initiate(inFile, &width, &height, &maxColor, &version);
  if (error){
    switch(error){
      case 1:
        fprintf(stderr, "ERROR: Invalid magic number detected\n");
        break;

      case 2:
        fprintf(stderr, "ERROR: No input file detected.\n");
        break;

      case 3:
        fprintf(stderr, "ERROR: Invalid header detected\n");
        break;
    }
    fclose(inFile);
    exit(1);
  }

  buffer = (pixel *) malloc(sizeof(pixel) * width * height);

  if (version == 3){
    readPPM3(inFile, buffer, &width, &height, &maxColor);
  }else{
    readPPM6(inFile, buffer, &width, &height, &maxColor);
  }

  fclose(inFile);

  if (!init()) {
    fprintf(stderr, "ERROR: Unable to open window.\n");
    exit(1);
  } else {
    run();
  }

  return 0;
}
