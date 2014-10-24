#include <AntTweakBar.h>

#define GLFW_INCLUDE_GLU
#define GL_GLEXT_PROTOTYPES
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#if defined(__APPLE_CC__)
#include <OpenGL/glext.h>
#else
#  include <GL/glext.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "shader.h"
#include <string>
#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>

//Location of vertex position in shader
#define  POSITION_ATTRIB 0

//The maximum number of control pts allowed in the color map
#define MAX_PTS 10


/* The width and height of the window.
 * Changed when the window is resized.
*/
int height = 480;
int width = 640;

/*The width and height of the loaded image*/
int pic_width;
int pic_height;

TwBar *bar;
//File name procided by the user
std::string fileName;
std::string fileOut = "out";

//An orientation image passed to the vertex shader.
//Used to adjust the viewing area with respect to the window size.
float orient[9] = {
  1,0,0,
  0,1,0,
  0,0,0
};

/* 0: 
 * 1:
 * 2:
 * 3:
 * 4:
 */
GLuint shaderUniforms[5];

//Main Window
GLFWwindow* window;

//Rectangle which covers the viewport
GLfloat vert_data[] = {
  -1.0, -1.0, 0.0,
  1.0, -1.0, 0.0,
  1.0, 1.0, 0.0,
  -1.0,1.0,0.0
};

//Passed as clientData in callbacks to identifiy buttons
int counting_numbers[] = { 0,1,2,3,4,5,6,7,8,9,10};

int pts_used = 0; //The number of active control points
float ctrl_pts[MAX_PTS]; //value of each control point
// The r,g,b values corresponding to each control point
// I.E. ctrl_colors[3] is the red value for the 2nd control point
float ctrl_colors[3*MAX_PTS];

//Prototypes
void init_matrix();
void identity_matrix();

//Called when the fileName string is modified in the UI
void TW_CALL SetMyStdStringCB(const void *value, void *clientData)
{
  //Some crazy memory managment.
  //Taken from an example in the TW documentation
  const std::string *srcPtr = static_cast<const std::string *>(value);
  fileOut = *srcPtr;
}

//Called when the fileName string in the UI needs to be read by TW
void TW_CALL GetMyStdStringCB(void *value, void * /*clientData*/)
{
  //See the same example
  std::string *destPtr = static_cast<std::string *>(value);
  TwCopyStdStringToLibrary(*destPtr, fileOut);
}

//Called when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
  width = w; //Keep track of the current window size
  height = h;
  glViewport(0, 0, width, height);
  init_matrix(); //Change the matrix to reflect the new aspect ratio
  TwWindowSize(width,height);
}

void MouseButtonCB( GLFWwindow*,int button ,int action ,int mods)
{
	TwEventMouseButtonGLFW( button , action );
}

void MousePosCB(GLFWwindow*,double x ,double y)
{
	TwEventMousePosGLFW( (int)x, (int)y );
}

void KeyFunCB( GLFWwindow* window,int key,int scancode,int action,int mods)
{
	TwEventKeyGLFW( key , action );
	TwEventCharGLFW( key  , action );
}

void MouseScrollCB(  GLFWwindow* window, double x , double y )
{
	TwEventMouseWheelGLFW( (int)y );
}

//An attempt to check the existance of a shader file before opening it.
//Dosent work as of yet.

/*inline bool file_exists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
  }*/

/*Parse the arguments from the command line*/
bool parse_args(int num, char** args){

  if(num < 2){
    std::cerr << "you must provide a file" << std::endl;
    exit(0);
  }

  fileName = std::string(args[1]);

  return true;

}

GLuint nrrd_get_type(int type){
  switch(type){
  case nrrdTypeChar:
    return GL_BYTE;
  case nrrdTypeUChar:
    return GL_UNSIGNED_BYTE;
  case nrrdTypeShort:
    return GL_SHORT;
  case nrrdTypeUShort:
    return GL_UNSIGNED_SHORT;
  case nrrdTypeInt:
    return GL_INT;
  case nrrdTypeUInt:
    return GL_UNSIGNED_INT;
  case nrrdTypeFloat:
    return GL_FLOAT;
  case nrrdTypeDouble:
    return GL_DOUBLE;
  default:
    return 0;

  }

}

static void error_callback(int error, const char* description){
  fputs(description, stderr);
}

//Render the main image (without the UI)
void render(void){
  //Update shader uniforms
  glUniform1fv(shaderUniforms[1],MAX_PTS,ctrl_pts);
  glUniform3fv(shaderUniforms[2],MAX_PTS,ctrl_colors);
  glUniform1i(shaderUniforms[3],pts_used);
  glUniformMatrix3fv(shaderUniforms[4],1,false,orient);

  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLE_FAN,0,4);

}

/* Takes a screenshot of the current colormap and saves it into
 * a file. Preserves the resolution of the origional 
*/
void take_screenshot(void* clientData){
  
  GLuint fbo, rboColor, rboDepth;

  /* Create the buffers to do a rendering pass into*/
  glGenRenderbuffers(1, &rboColor);
  glBindRenderbuffer(GL_RENDERBUFFER,rboColor);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, 
			pic_width,pic_height);

  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER,rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
			pic_width,pic_height);

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, 
			    GL_COLOR_ATTACHMENT0,
			    GL_RENDERBUFFER,
			    rboColor);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, 
			    GL_DEPTH_ATTACHMENT,
			    GL_RENDERBUFFER,
			    rboDepth);
  //Bind the new buffers to the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0); 

  //Set the resolution to that given by the pic.
  glViewport(0,0,pic_width,pic_height);

  /*Set the transformation matrix to do nothing so that 
    the image fills the viewport*/
  identity_matrix();
 
  /*Render the image into the buffer*/
  render();

  //Reset the the size of the viewport
  glViewport(0,0,width,height);

  //The rendered image is read into ss
  GLvoid *ss = malloc(sizeof(unsigned char)*3*pic_width*pic_height);
  glReadPixels(0,0,pic_width,pic_height,GL_RGB,
	       GL_UNSIGNED_BYTE,ss);

  //Used to save the rendered image
  Nrrd *nflip = nrrdNew();
  Nrrd *nout = nrrdNew();

  char* err;
    //Saves the contents of the buffer as a png image
    //Currently upsidedown

if (nrrdWrap_va(nflip, ss, nrrdTypeUChar, 3,
		(size_t)3, (size_t)pic_width, (size_t)pic_height)
    || nrrdFlip(nout,nflip, 2) ||  nrrdSave((fileOut + std::string(".png")).c_str(), nout, NULL)) {
      err = biffGetDone(NRRD);
      fprintf(stderr, "trouble wrapping image:\n%s", err);
      free(err);
      return;
    }

 nrrdNuke(nout);
 nrrdNuke(nflip);


 //Reset the transformation matrix
 init_matrix();

  //rebind the frambuffer to the screen
 glBindFramebuffer (GL_FRAMEBUFFER, 0);
}

/* Called when MvUP button is press for a given control-point 
 * moves the value/color of a control point up the list of points
 */
void TWCB_MvUP(void* clientData){
  int b = *((int*)clientData); //The number of the control point
  float v,c1,c2,c3;

  v = ctrl_pts[b-1]; //value of the control point

  if(b == 1) //the first control point is always 0
    ctrl_pts[b-1] = 0.0;
  else
    ctrl_pts[b-1] = ctrl_pts[b]; //Move bth to the (b-1)th control point

  if(b == pts_used)//Last point is always 1.0
    ctrl_pts[b] = 1.0;
  else
    ctrl_pts[b] = v;//move the (b-1)th point into the (bth) point

  /*Swap the colors */
  c1 = ctrl_colors[(b-1)*3];
  ctrl_colors[(b-1)*3] = ctrl_colors[b*3];
  ctrl_colors[b*3] = c1;

  c2 = ctrl_colors[(b-1)*3+1];
  ctrl_colors[(b-1)*3+1] = ctrl_colors[b*3+1];
  ctrl_colors[b*3+1] = c2;

  c3 = ctrl_colors[(b-1)*3+2];
  ctrl_colors[(b-1)*3+2] = ctrl_colors[b*3+2];
  ctrl_colors[b*3+2] = c3;

}

/* Add a control point in the color map.
 * Control point is added at the end of the list
 */
void TWCB_AddCtrlPt( void* clientData){
  //Max number of points must be chosen at the time the shader is compiled
  if(pts_used == MAX_PTS)
    return;

  //Unlock the (previously) maximum point so that is no longer locked at 1.0
  if(pts_used != 0){
    std::string access = "TweakBar/";
    access += std::string("CtrlPt") + std::to_string(pts_used);
    access += std::string(" min=0.0");

    TwDefine(access.c_str());
  }

  //Update data
  pts_used++;
  ctrl_pts[pts_used] = 1.0;
  ctrl_colors[pts_used*3] = 0.0;
  ctrl_colors[pts_used*3+1] = 0.0;
  ctrl_colors[pts_used*3 + 2] = 0.0;
  
  //Add the point value, color, and mvUp button
  std::string num = std::to_string(pts_used);
  
  std::string def_str = std::string("min=1.0 max=1.0 step=.001 group=") + num;
  TwAddVarRW(bar, (std::string("CtrlPt") + num).c_str(),
	     TW_TYPE_FLOAT, ctrl_pts + pts_used, def_str.c_str());
  
  TwAddVarRW(bar, (std::string("Color") + num).c_str(),
	     TW_TYPE_COLOR3F, ctrl_colors + (3*pts_used), 
	     (std::string("group=")+num).c_str()); 

  TwAddButton(bar, (std::string("MvUp") + num).c_str(), TWCB_MvUP,
	      counting_numbers+pts_used, (std::string("group=")+num).c_str());
       
  

}

/* Removes the maximum control point */
void TWCB_RemoveCtrlPt(void* clientData){
  if(pts_used == 0)
    return;

  std::string num = std::to_string(pts_used);

  //Access and remove the pt value, color, and mvup button
  TwRemoveVar(bar, (std::string("CtrlPt") + num).c_str());
  TwRemoveVar(bar, (std::string("Color") + num).c_str());

  TwRemoveVar(bar, (std::string("MvUp") + num).c_str());

  pts_used--;

  //Lock the (newfound) largest point to 1.0
  std::string access = "TweakBar/";
  access += std::string("CtrlPt") + std::to_string(pts_used);
  access += std::string(" max=1.0 min=1.0");
  if(pts_used != 0)
    ctrl_pts[pts_used] = 1.0;

  TwDefine(access.c_str());



}

/* Initialize the UI using AntTweakBar */
void init_TW(){

  TwInit(TW_OPENGL_CORE, NULL);

  TwWindowSize(width,height);

  bar = TwNewBar("TweakBar");
  TwDefine(" TweakBar size='200 300' ");
  TwDefine(" TweakBar resizable=true ");

  TwAddButton(bar, "AddCtrlPt", TWCB_AddCtrlPt, 
	      NULL, "label='Add Point'");

  TwAddButton(bar, "RemoveCtrlPt", TWCB_RemoveCtrlPt,
	      NULL, "label='Remove Point'");

  TwAddButton(bar, "ScreenShot", take_screenshot,
	      NULL, "label='Save Screenshot'");

  TwAddVarCB(bar, "Screenshot Name", TW_TYPE_STDSTRING, 
	     SetMyStdStringCB, GetMyStdStringCB, NULL, "");

  //Begins with one control point. This control point is locked at 0
  TwAddVarRW(bar, "CtrlPt0", TW_TYPE_FLOAT, ctrl_pts,"min=0.0 max=0.0 step=.001 group=0");
  TwAddVarRW(bar, "CtrlColor0",TW_TYPE_COLOR3F,ctrl_colors,"group=0");

  //Register callbacks.
  glfwSetMouseButtonCallback( window , MouseButtonCB );
  glfwSetCursorPosCallback( window , MousePosCB);
  glfwSetScrollCallback( window , MouseScrollCB );
  glfwSetKeyCallback(window , KeyFunCB);
}

//Initializes the vertex array and binds to the buffer
void init_vao(){

  GLuint vao;
  GLuint bufs[1];
  glGenVertexArrays(1,&vao);
  glGenBuffers(1,bufs);
  
  glBindVertexArray(vao);
  glEnableVertexAttribArray(POSITION_ATTRIB);

  glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), 
	       vert_data, GL_STATIC_DRAW);
  glVertexAttribPointer(POSITION_ATTRIB, 3, GL_FLOAT,GL_FALSE,0, 0);
}

/*Initializes the fsh/vsh and then returns the shader program object*/
ShaderProgram* init_shader(){
  ShaderProgram *shader = new ShaderProgram(); 

  shader->vertexShader("vertex.vsh");
  shader->fragmentShader("frag.fsh");

  glBindAttribLocation(shader->progId,POSITION_ATTRIB, "position");

  glLinkProgram(shader->progId);
  glUseProgram(shader->progId);

  return shader;

}

/*Initalizes the texture from the image name given at the command line
 * returns the texture unit that it is bound to
 */
GLuint init_texture(){

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
  GLuint tex_size = nrrd_get_type(nrdImg->type);

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

  return tex;
}

/* Initalizes the unifrom variables for the shaders */
void init_uniforms(ShaderProgram* shader){
  shaderUniforms[0] = shader->UniformLocation("TextureUniform");
  glUniform1i(shaderUniforms[0],0);

  shaderUniforms[1] = shader->UniformLocation("ctrl_pts");
  shaderUniforms[2] = shader->UniformLocation("ctrl_colors");
  shaderUniforms[3] = shader->UniformLocation("max_pt");

  shaderUniforms[4] = shader->UniformLocation("orientMat");
}

/*Set the oreintation matrix to display the image in 
 * the correct aspect ratio. It uses the width of the viewport,
 * and dervies the height from the aspect ratio.
 */
void init_matrix(){
  float w1 = ((float)pic_width)/width;
  float w2 = ((float)pic_height)/height;

  orient[0] = 1;
  orient[1] = 0;
  orient[2] = 0;
  orient[3] = 0;
  orient[4] = w2/w1;
  orient[5] = 0;
  orient[6] = 0;
  orient[7] = 0;
  orient[8] = 0;

}

/* Sets the orientation matrix to the identity*/
void identity_matrix(){
  orient[0] = 1;
  orient[1] = 0;
  orient[2] = 0;
  orient[3] = 0;
  orient[4] = 1;
  orient[5] = 0;
  orient[6] = 0;
  orient[7] = 0;
  orient[8] = 0;

}

//Initalize the UI, Vertex Data, shaders, texture, matrix and uniforms
void init(void){
  init_TW();

  init_vao();

  ShaderProgram* s = init_shader();

  init_texture();

  init_matrix();

  init_uniforms(s);

		     
}

int main(int numArgs, char** args){
  
  parse_args(numArgs,args);

  //Initialize the first control point to zero.
  ctrl_pts[0] = 0.0;
  ctrl_colors[0] = 0.0;
  ctrl_colors[1] = 0.0;
  ctrl_colors[2] = 0.0;

  glfwSetErrorCallback(error_callback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Use OpenGL Core v3.2
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


  window = glfwCreateWindow(width, height, "Simple example", 
			    NULL, NULL);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!window)
    {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
  glfwMakeContextCurrent(window);

  init();

  while(!glfwWindowShouldClose(window)){
    render();
    TwDraw();
    glfwWaitEvents();
    glfwSwapBuffers(window);

  }

 glfwDestroyWindow(window);
 glfwTerminate();
 exit(EXIT_SUCCESS);
}
