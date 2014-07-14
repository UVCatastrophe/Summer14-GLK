#include <AntTweakBar.h>

#define GLFW_INCLUDE_GLU
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "shader.h"
#include "SOIL/SOIL.h"
//#include "CImg.h"

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

/*Main display window*/
GLFWwindow* window;

/* A rectangle which encompases the entire viewing area when 
 * drawn with GL_TRINAGLE_FAN */
GLfloat vert_data[] = {
  -1.0, -1.0, 0.0,
  1.0, -1.0, 0.0,
  1.0, 1.0, 0.0,
  -1.0,1.0,0.0
};

/* Called when the window size is modified */
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
  glViewport(0, 0, w, h); //Update the viewport
  TwWindowSize(w,h); //Notify AntTweakBar of the Update
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

  glDrawArrays(GL_TRIANGLE_FAN,0,4);

}

/*Initializes the UI using AntTweakBar */
void init_TW(){

  TwInit(TW_OPENGL, NULL);

  TwWindowSize(640,480);

  //The Main Bar used throughout
  TwBar *bar = TwNewBar("TweakBar");
  TwDefine(" TweakBar size='200 300' ");
  TwDefine(" TweakBar resizable=false ");

  //The two colors which will be interpolated between
  TwAddVarRW(bar, "Color1", TW_TYPE_COLOR3F, color1,
	     " label='Color1' colormode=hls");

  TwAddVarRW(bar, "Color2", TW_TYPE_COLOR3F, color2,
	     " label='Color2'");

  //Register callbacks for GLFW 
  glfwSetMouseButtonCallback( window , MouseButtonCB );
  glfwSetCursorPosCallback( window , MousePosCB);
  glfwSetScrollCallback( window , MouseScrollCB );
  glfwSetKeyCallback(window , KeyFunCB);
}

/*Initializes the the opengl Context, VAO's, shaders, and UI */
void init(void){

  init_TW();

  //-------Load the Vertex data-------
  GLuint vao;
  GLuint bufs[1];
  glGenVertexArrays(1,&vao);
  glGenBuffers(1,bufs);
  
  glBindVertexArray(vao);
  glEnableVertexAttribArray(POSITION_ATTRIB);
  glEnableVertexAttribArray(COLOR_ATTRIB);

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
  glActiveTexture(GL_TEXTURE0);

  GLuint tex = SOIL_load_OGL_texture(
				     fileName.c_str(),
				     SOIL_LOAD_AUTO,
				     SOIL_CREATE_NEW_ID,
				     SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
				     );

  if(tex == 0){
    std::cout << "error loading texture" << std::endl;
    exit(0);
  }

  glBindTexture(GL_TEXTURE_2D,tex);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


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

  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  init();

  while(!glfwWindowShouldClose(window)){
    render();//Render the image
    TwDraw(); //Render AntTweakBar
    glfwWaitEvents(); //Only render a frame when a change is made
    glfwSwapBuffers(window);

  }

  //Clean up
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
