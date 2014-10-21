#include <AntTweakBar.h>

#define GLFW_INCLUDE_GLU
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "shader.h"
#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>

#define POSITION_ATTRIB 0
#define COLOR_ATTRIB 1

//Pathname for the vertex and fragment shaders
#define FSH_FILE "frag.fsh"
#define VSH_FILE "vertex.vsh"

/* Path of the monochrome image. Loaded from command line. */
std::string fileName;

/*The two colors which will be interpolated between */
GLfloat color1[3] = {1.0,0.0,0.0};
GLfloat color2[3] = {0.5,0.5,0.5};

/* 0 : textureUniform
 * 1 : color1
 * 2 : color2
 * 3 : Unused
 */
GLuint shaderUniforms[4];

GLuint vao;
GLuint bufs[1];

/*Main display window*/
GLFWwindow* window;

/*Secondary Display Window for ATB*/
GLFWwindow* windowATB;

/* A rectangle which encompases the entire viewing area when 
 * drawn with GL_TRINAGLE_FAN */
GLfloat vert_data[] = {
  -1.0, -1.0, 0.0,
  1.0, -1.0, 0.0,
  1.0, 1.0, 0.0,
  -1.0,1.0,0.0
};

/* Called when the window size is modified */
void framebuffer_size_callback(GLFWwindow* win, int w, int h)
{
  GLFWwindow *current = glfwGetCurrentContext();

  if(win != current)
    glfwMakeContextCurrent(win);
  glViewport(0, 0, w, h); //Update the viewport
  if(win == windowATB)
    TwWindowSize(w,h); //Notify AntTweakBar of the Update

  if(win != current)
    glfwMakeContextCurrent(current);
}

/* Called when the mouse button is clicked*/
void MouseButtonCB( GLFWwindow*,int button ,int action ,int mods)
{
  //Pass to AntTweakBar
  TwEventMouseButtonGLFW( button , action );
}

/*Called when the mouse is moved*/
void MousePosCB(GLFWwindow*,double x ,double y)
{
  //Pass to AntTweakBar
  TwEventMousePosGLFW( (int)x, (int)y );
}

/*Called when a key is pressed*/
void KeyFunCB( GLFWwindow* window,int key,int scancode,int action,int mods)
{
  TwEventKeyGLFW( key , action );
  TwEventCharGLFW( key  , action );
}

/*Called when the scroll wheel is used*/
void MouseScrollCB(  GLFWwindow* window, double x , double y )
{
  TwEventMouseWheelGLFW( (int)y );
}

/* Called when opengl encounters an error*/
static void error_callback(int error, const char* description){
  fputs(description, stderr);
}

/* Parse the arguments from the command line. 
 * Set the fileName to the first argument */
bool parse_args(int num, char** args){

  if(num < 2){
    std::cout << "You must provide a file name" << std::endl;
    exit(0);
  }

  fileName = std::string(args[1]);

  return true;

}

void render(void){

  glUniform3f(shaderUniforms[1],color1[0],color1[1],color1[2]);
  glUniform3f(shaderUniforms[2],color2[0],color2[1],color2[2]);

  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(vao);

  glDrawArrays(GL_TRIANGLE_FAN,0,4);

}

/*Initializes the UI using AntTweakBar */
void init_TW(){
  glfwMakeContextCurrent(windowATB);

  TwInit(TW_OPENGL, NULL);

  TwWindowSize(200,300);

  //The Main Bar used throughout
  TwBar *bar = TwNewBar("TweakBar");
  TwDefine(" TweakBar size='200 300' ");
  TwDefine(" TweakBar resizable=false movable=false contained=true alpha=255 position='0 0'");

  //The two colors which will be interpolated between
  TwAddVarRW(bar, "Color1", TW_TYPE_COLOR3F, color1,
	     " label='Color1' colormode=hls");

  TwAddVarRW(bar, "Color2", TW_TYPE_COLOR3F, color2,
	     " label='Color2'");

  //Register callbacks for GLFW 
  glfwSetMouseButtonCallback( windowATB , MouseButtonCB );
  glfwSetCursorPosCallback( windowATB , MousePosCB);
  glfwSetScrollCallback( windowATB , MouseScrollCB );
  glfwSetKeyCallback(windowATB , KeyFunCB);
  glfwSetFramebufferSizeCallback(windowATB,framebuffer_size_callback);

  glfwMakeContextCurrent(window);
}

/*Initializes the the opengl Context, VAO's, shaders, and UI */
void init(void){

  init_TW();

  //-------Load the Vertex data-------
  glGenVertexArrays(1,&vao);
  glGenBuffers(1,bufs);
  
  glBindVertexArray(vao);
  glEnableVertexAttribArray(POSITION_ATTRIB);
  // glEnableVertexAttribArray(COLOR_ATTRIB);

  glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), 
	       vert_data, GL_STATIC_DRAW);
  glVertexAttribPointer(POSITION_ATTRIB, 3, GL_FLOAT,GL_FALSE,0, 0);

  //-------Load the vertex and fragment shader----------
  ShaderProgram *shader = new ShaderProgram(); 

  shader->vertexShader(VSH_FILE);
  shader->fragmentShader(FSH_FILE);

  glBindAttribLocation(shader->progId,POSITION_ATTRIB, "position");

  glLinkProgram(shader->progId);
  glUseProgram(shader->progId);

  //------Load the given image as a texture----------
  //Use teem to load the image
  Nrrd *nrdImg = nrrdNew();

  if(nrrdLoad(nrdImg,fileName.c_str(),NULL)){
    char* err = biffGetDone(NRRD);
    fprintf(stderr, "trouble reading \"%s\":\n%s", 
	    fileName.c_str(), err);
    free(err);
    exit(1);
  }

  GLuint tex;
  glGenTextures(1, &tex);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,tex);

  GLuint tex_mode;
  GLuint tex_size = GL_UNSIGNED_BYTE;
  int pic_width;
  int pic_height;

  if( nrdImg->dim == 2){
    pic_width = nrdImg->axis[0].size;
    pic_height = nrdImg->axis[1].size;

    tex_mode = GL_RED;
  }

  else{
    pic_width = nrdImg->axis[1].size;
    pic_height = nrdImg->axis[2].size;

    switch(nrdImg->axis[0].size){
    case 3:
      tex_mode = GL_RGB;
      break;
    case 4:
      tex_mode = GL_RGBA;
      break;
    }

  }

  glTexImage2D(GL_TEXTURE_2D, 0, tex_mode, pic_width, 
	       pic_height, 0, tex_mode, tex_size, 
	       nrdImg->data);

  int e;
  if((e = glGetError()) != GL_NO_ERROR)
    std::cout << e << std::endl;

  nrrdNuke(nrdImg);

  glBindTexture(GL_TEXTURE_2D,tex);
  
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		   GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		   GL_LINEAR );


  //Activate shader Uniforms
  shaderUniforms[0] = shader->UniformLocation("TextureUniform");
  glUniform1i(shaderUniforms[0],0);

  shaderUniforms[1] = shader->UniformLocation("color1");

  shaderUniforms[2] = shader->UniformLocation("color2");

}

/*Initialize opengl context/data and then enter render loop*/
int main(int numArgs, char** args){
  
  parse_args(numArgs,args);

  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);

  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

  windowATB = glfwCreateWindow(200,300, "ATB", NULL,NULL);
  if(!windowATB){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  init();

  while(!glfwWindowShouldClose(window)){
    glfwMakeContextCurrent(window);
    render();//Render the image
    glfwMakeContextCurrent(windowATB);
    TwDraw(); //Render AntTweakBar
    glfwWaitEvents(); //Only render a frame when a change is made
    glfwSwapBuffers(windowATB);
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);

  }

  //Clean up
  glfwDestroyWindow(window);
  glfwDestroyWindow(windowATB);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
