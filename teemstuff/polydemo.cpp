
/*

to compile:
gcc -Wall -o polydemo polydemo.c -I/$TEEM/include -L/$TEEM/lib -Wl,-rpath,$TEEM/lib -lteem -lpng

where $TEEM is the path into your teem-install directory
(with bin, lib, and include subdirectories)

example run: ./polydemo -r 10 20 -p 0 1 2 4
which uses two different polygonal resolutions (10, 20),
and 4 different values of beta in the superquadric shape (0, 1, 2, 4)

 */
#include <AntTweakBar.h>

#define GLM_FORCE_RADIANS
#define GLFW_INCLUDE_GLU
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>
#include <teem/limn.h>
#include "shader.h"
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream> 

/*Dimensions of the screen*/
double height = 480;
double width = 640;

float lpd_alpha = .5;
float lpd_beta = 0;
unsigned int lpd_theta = 10;
unsigned int lpd_phi = 20;

unsigned int old_indxNum;

limnPolyData *poly;

struct render_info{
  GLuint vao = -1;
  GLuint buffs[3];
  GLuint uniforms[3];
  GLuint elms;
  ShaderProgram* shader = NULL;
} render;

struct ui_pos{
  bool isDown = false;
  int mode; //0 for rotate, 1 for zoom
  double last_x;
  double last_y;
} ui;

struct camera{
  glm::vec3 center = glm::vec3(0.0f,0.0f,0.0f);
  glm::vec3 pos = glm::vec3(3.0f,0.0f,0.0f);
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
} cam;

glm::mat4 view = glm::lookAt(cam.pos,cam.center,cam.up);
glm::mat4 proj = glm::perspective(1.0f, ((float) width)/((float)height),.1f, 100.0f);
glm::mat4 model = glm::mat4();

void mouseButtonCB(GLFWwindow* w, int button, 
		   int action, int mods){
  TwEventMouseButtonGLFW( button , action );

  glfwGetCursorPos (w, &(ui.last_x), &(ui.last_y));

  if(action == GLFW_PRESS){
    ui.isDown = true;
  }
  else if(action == GLFW_RELEASE){
    ui.isDown = false;
  }

  if(ui.last_x <= width*.1)
    ui.mode = 1;
  else
    ui.mode = 0;
}

void mousePosCB(GLFWwindow* w, double x, double y){
  TwEventMousePosGLFW( (int)x, (int)y );

  if(!ui.isDown)
    return;

  if(ui.mode == 0){
    float x_diff = ui.last_x - x;
    float y_diff = ui.last_y - y;
    glm::vec3 norm = glm::cross(cam.pos-cam.center, glm::vec3(0,y_diff,x_diff));
    float angle = (glm::length(glm::vec2(x_diff,y_diff)) * 2*3.1415 ) / width;
    
    model = glm::rotate(glm::mat4(),angle,norm)* model;
    
    ui.last_x = x;
    ui.last_y = y;
  }
  
  else if(ui.mode == 1){
    float y_diff = ui.last_y - y;
    cam.pos += glm::normalize(cam.pos-cam.center) * y_diff*.001f;

    view = glm::lookAt(cam.pos,cam.center,cam.up);

    ui.last_x = x;
    ui.last_y = y;
  }

}

void screenSizeCB(GLFWwindow* win, int w, int h){
  width = w;
  height = h;

  glViewport(0,0,width,height);

  proj = glm::perspective(1.0f, ((float) width)/((float)height),.1f, 100.0f);

}

void keyFunCB( GLFWwindow* window,int key,int scancode,int action,int mods)
{
  TwEventKeyGLFW( key , action );
  TwEventCharGLFW( key  , action );
}

void mouseScrollCB(  GLFWwindow* window, double x , double y )
{
  TwEventMouseWheelGLFW( (int)y );
}

const char *pdInfo=("Program to demo limnPolyData (as well as the "
"hest command-line option parser)");

/* Converts a teem enum to an openGL enum */
GLuint get_prim(unsigned char type){
  switch(type){
  case limnPrimitiveUnknown:
    return 0;
  case limnPrimitiveNoop:
    return 0;
  case limnPrimitiveTriangles:
    return GL_TRIANGLES;
  case limnPrimitiveTriangleStrip:
    return GL_TRIANGLE_STRIP;
  case limnPrimitiveTriangleFan:
    return GL_TRIANGLE_FAN;
  case limnPrimitiveQuads:
    return GL_QUADS;
  case limnPrimitiveLineStrip:
    return GL_LINE_STRIP;
  case limnPrimitiveLines:
    return GL_LINES;
  case limnPrimitiveLast:
    return 0;
  }

}

limnPolyData *generate_spiral(float A, float B,unsigned int thetaRes,
			      unsigned int phiRes){
  airArray *mop;

  char *err;
  unsigned int *res, resNum, resIdx, parmNum, parmIdx, flag;
  float *parm;
  limnPolyData *lpd;

  lpd = limnPolyDataNew();
  /* this controls which arrays of per-vertex info will be allocated
     inside the limnPolyData */
  flag = ((1 << limnPolyDataInfoRGBA)
          | (1 << limnPolyDataInfoNorm));

  /* this creates the polydata, re-using arrays where possible
     and allocating them when needed */
  if (limnPolyDataSpiralSuperquadric(lpd, flag,
				     A, B, /* alpha, beta */
				     thetaRes, phiRes)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "trouble making polydata:\n%s", err);
    airMopError(mop);
    return NULL;
  }

  /* do something with per-vertex data to make it visually more interesting:
     the R,G,B colors increase with X, Y, and Z, respectively */
  unsigned int vertIdx;
  for (vertIdx=0; vertIdx<lpd->xyzwNum; vertIdx++) {
    float *xyzw = lpd->xyzw + 4*vertIdx;
    unsigned char *rgba = lpd->rgba + 4*vertIdx;
    rgba[0] = AIR_CAST(unsigned char, AIR_AFFINE(-1, xyzw[0], 1, 40, 255));
    rgba[1] = AIR_CAST(unsigned char, AIR_AFFINE(-1, xyzw[1], 1, 40, 255));
    rgba[2] = AIR_CAST(unsigned char, AIR_AFFINE(-1, xyzw[2], 1, 40, 255));

    rgba[3] = 255;
  }
  
  return lpd;
}

void render_poly(){
  glUniformMatrix4fv(render.uniforms[0],1,false,glm::value_ptr(proj));
  glUniformMatrix4fv(render.uniforms[1],1,false,glm::value_ptr(view));
  glUniformMatrix4fv(render.uniforms[2],1,false,glm::value_ptr(model));
  
  glClear(GL_DEPTH_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT);
  int offset = 0;
  for(int i = 0; i < poly->primNum; i++){
    GLuint prim = get_prim(poly->type[i]);
    
    glDrawElements( prim, poly->icnt[i], 
		    GL_UNSIGNED_INT, ((void*) 0) + offset);
    offset += poly->icnt[i];
    GLuint error;
    if( (error = glGetError()) != GL_NO_ERROR)
      std::cout << "GLERROR: " << error << std::endl;
  }
  
}

void buffer_data(bool bufferIndicies){
  if(render.vao == -1){

    glGenVertexArrays(1, &(render.vao));
    glGenBuffers(3,render.buffs);
    
    glBindVertexArray(render.vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
  }

  //Verts
  glBindBuffer(GL_ARRAY_BUFFER, render.buffs[0]);
  glBufferData(GL_ARRAY_BUFFER, poly->xyzwNum*sizeof(float)*4,
	       poly->xyzw, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT,GL_FALSE,0, 0);

  //Norms
  glBindBuffer(GL_ARRAY_BUFFER, render.buffs[1]);
  glBufferData(GL_ARRAY_BUFFER, poly->normNum*sizeof(float)*3,
	       poly->norm, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT,GL_FALSE,0, 0);

  //Colors
  glBindBuffer(GL_ARRAY_BUFFER, render.buffs[2]);
  glBufferData(GL_ARRAY_BUFFER, poly->rgbaNum*sizeof(char)*4,
	       poly->rgba, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(2, 4, GL_BYTE,GL_FALSE,0, 0);

  if(bufferIndicies){
    //Indices
    glGenBuffers(1, &(render.elms));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render.elms);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
		 poly->indxNum * sizeof(unsigned int),
		 poly->indx, GL_DYNAMIC_DRAW);
  }
}

void enable_shaders(const char* vshFile, const char* fshFile){
  //Initialize the shaders
  render.shader = new ShaderProgram(); 
  
  //Set up the shader
  render.shader->vertexShader(vshFile);
  render.shader->fragmentShader(fshFile);
  
  glBindAttribLocation(render.shader->progId,0, "position");
  glBindAttribLocation(render.shader->progId,1, "norm");
  glBindAttribLocation(render.shader->progId,2, "color");
  
  glLinkProgram(render.shader->progId);
  glUseProgram(render.shader->progId);
  
  render.uniforms[0] = render.shader->UniformLocation("proj");
  render.uniforms[1] = render.shader->UniformLocation("view");
  render.uniforms[2] = render.shader->UniformLocation("model");
  
}

void TWCB_Update(void* clientData){
  poly = generate_spiral(lpd_alpha,lpd_beta,lpd_theta,lpd_phi);
  buffer_data(true);
  old_indxNum = poly->indxNum;

}

void init_ATB(){
  TwInit(TW_OPENGL, NULL);

  TwWindowSize(width,height);

  TwBar *bar = TwNewBar("lpdTweak");

  TwDefine(" lpdTweak size='200 200' ");
  TwDefine(" lpdTweak resizable=true ");

  TwAddVarRW(bar, "ALPHA", TW_TYPE_FLOAT, &lpd_alpha, 
	     "step=.01 label=Alpha");

  TwAddVarRW(bar, "BETA", TW_TYPE_FLOAT, &lpd_beta,
	     "step=.01 label=Beta");

  TwAddVarRW(bar, "THETA_RES", TW_TYPE_UINT32, &lpd_theta, "label='Theta Resolution'");

  TwAddVarRW(bar, "PHI_RES", TW_TYPE_UINT32, &lpd_phi, "label='Phi Resolution'");

  TwAddButton(bar, "UPDATE",  TWCB_Update, NULL, "label=update");

}


int main(int argc, const char **argv) {
  
  glfwInit();

  GLFWwindow *window = glfwCreateWindow(640,480, "Sample", NULL, NULL);
  if(!window){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  
  init_ATB();

  enable_shaders("shader.vsh","shader.fsh");

  poly = generate_spiral(lpd_alpha,lpd_beta, lpd_theta, lpd_phi);
  buffer_data(true);
  old_indxNum = poly->indxNum;

  glBindVertexArray(render.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render.elms);

  //Now render the object
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

  glfwSetCursorPosCallback(window, mousePosCB);
  glfwSetMouseButtonCallback(window,mouseButtonCB);
  glfwSetWindowSizeCallback(window,screenSizeCB);
  glfwSetScrollCallback( window , mouseScrollCB );
  glfwSetKeyCallback(window , keyFunCB);


  glEnable(GL_DEPTH_TEST);

  while(true){
    render_poly();

    TwDraw();
    glfwWaitEvents();
    glfwSwapBuffers(window);

  }

  exit(0);
}
