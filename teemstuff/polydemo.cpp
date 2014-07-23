
/*

to compile:
gcc -Wall -o polydemo polydemo.c -I/$TEEM/include -L/$TEEM/lib -Wl,-rpath,$TEEM/lib -lteem -lpng

where $TEEM is the path into your teem-install directory
(with bin, lib, and include subdirectories)

example run: ./polydemo -r 10 20 -p 0 1 2 4
which uses two different polygonal resolutions (10, 20),
and 4 different values of beta in the superquadric shape (0, 1, 2, 4)

 */


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

int main(int argc, const char **argv) {
  const char *me;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;

  char *err;
  unsigned int *res, resNum, resIdx, parmNum, parmIdx, flag;
  float *parm;
  limnPolyData *lpd;

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "r", "r0 r1", airTypeUInt, 1, -1, &res, "10",
             "polydata resolutions to use", &resNum);
  hestOptAdd(&hopt, "p", "p0 p1", airTypeFloat, 1, -1, &parm, "1.0",
             "parameters to use", &parmNum);
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, pdInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);


  lpd = limnPolyDataNew();
  airMopAdd(mop, lpd, (airMopper)limnPolyDataNix, airMopAlways);
  /* this controls which arrays of per-vertex info will be allocated
     inside the limnPolyData */
  flag = ((1 << limnPolyDataInfoRGBA)
          | (1 << limnPolyDataInfoNorm));

  //#if 0
  /* We're looping through these values not because its an especially
     important example, but it provides an example of changing
     polydata in a way that does *not* change the number and
     connectivity of vertices (inner loop), and in a way that *does*
     change the number and connectivity of vertices (outer loop) */
  for (resIdx=0; resIdx<resNum; resIdx++) {
    printf("res[%u] = %u\n", resIdx, res[resIdx]);
    for (parmIdx=0; parmIdx<parmNum; parmIdx++) {
      unsigned vertIdx, primIdx;
      printf("    parm[%u] = %g\n", parmIdx, parm[parmIdx]);

      /* this creates the polydata, re-using arrays where possible
         and allocating them when needed */
      if (limnPolyDataSpiralSuperquadric(lpd, flag,
					 0.5, parm[parmIdx], /* alpha, beta */
					 2*res[resIdx], res[resIdx])) {
	airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
	fprintf(stderr, "%s: trouble making polydata:\n%s", me, err);
	airMopError(mop);
	return 1;
      }

      /* this (textually) describes the polydata: lpd->xyzw, lpd->norm,
         lpd->rgba represent the per-vertex data. The polydata can be one or
         more "primitives", like a triangle strip: the values in the
         lpd->type[] array comes from the limnPrimitive*enum */
      printf("        info for vertices (%u indices in %p):\n",
             lpd->indxNum, lpd->indx);
      printf("        xyzw=%p (# %u), norm=%p (# %u), rgba=%p (# %u)\n",
             lpd->xyzw, lpd->xyzwNum,
             lpd->norm, lpd->normNum,
             lpd->rgba, lpd->rgbaNum);
      printf("        %u primitive(s)\n", lpd->primNum);
      for (primIdx=0; primIdx<lpd->primNum; primIdx++) {
        printf("            prim[%u]: %s (type %u) has %u vertex indices\n", primIdx,
               airEnumStr(limnPrimitive, lpd->type[primIdx]), lpd->type[primIdx],
               lpd->icnt[primIdx]);
      }

      /* do something with per-vertex data to make it visually more interesting:
         the R,G,B colors increase with X, Y, and Z, respectively */
      for (vertIdx=0; vertIdx<lpd->xyzwNum; vertIdx++) {
        float *xyzw = lpd->xyzw + 4*vertIdx;
        unsigned char *rgba = lpd->rgba + 4*vertIdx;
        rgba[0] = AIR_CAST(unsigned char, AIR_AFFINE(-1, xyzw[0], 1, 20, 255));
        rgba[1] = AIR_CAST(unsigned char, AIR_AFFINE(-1, xyzw[1], 1, 20, 255));
        rgba[2] = AIR_CAST(unsigned char, AIR_AFFINE(-1, xyzw[2], 1, 20, 255));
      }

      /* Can lpd now be rendered? */

    }
  }
  //#endif

  //limnPolyDataCube(lpd,flag,0);

  glfwInit();

  GLFWwindow *window = glfwCreateWindow(640,480, "Sample", NULL, NULL);
  if(!window){
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  //Initialize the vao/vbo
  GLuint vao;
  // 0: Vertices
  // 1: Normals
  // 2: Colors
  GLuint bufs[3];

  glGenVertexArrays(1, &vao);
  glGenBuffers(3,bufs);

  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  //Verts
  glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
  glBufferData(GL_ARRAY_BUFFER, lpd->xyzwNum*sizeof(float)*4,
	       lpd->xyzw, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT,GL_FALSE,0, 0);

  //Norms
  glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
  glBufferData(GL_ARRAY_BUFFER, lpd->normNum*sizeof(float)*3,
	       lpd->norm, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT,GL_FALSE,0, 0);

  //Colors
  glBindBuffer(GL_ARRAY_BUFFER, bufs[2]);
  glBufferData(GL_ARRAY_BUFFER, lpd->rgbaNum*sizeof(char)*4,
	       lpd->rgba, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 4, GL_BYTE,GL_FALSE,0, 0);



  //Indices
  GLuint elms;
  glGenBuffers(1, &elms);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elms);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
	       lpd->indxNum * sizeof(unsigned int),
	       lpd->indx, GL_STATIC_DRAW);


  //Initialize the shaders
  ShaderProgram *shader = new ShaderProgram(); 

  const char* VSH_FILE = "shader.vsh";
  const char* FSH_FILE = "shader.fsh";

  //Set up the shader
  shader->vertexShader(VSH_FILE);
  shader->fragmentShader(FSH_FILE);

  glBindAttribLocation(shader->progId,0, "position");
  glBindAttribLocation(shader->progId,1, "norm");
  glBindAttribLocation(shader->progId,2, "color");

  glLinkProgram(shader->progId);
  glUseProgram(shader->progId);

  /*
  for(int i = 0; i < lpd->xyzwNum; i++){
    float x = lpd->xyzw[0 + 4*i];
    float y = lpd->xyzw[1 + 4*i];
    float z = lpd->xyzw[2 + 4*i];
    float w = lpd->xyzw[3 + 4*i];
     glm::vec4 v = glm::vec4(x,y,z,w);
     v = (proj * view * v);
     v = v / v.w;
     std::cout << i << ": " << v.x << " " << v.y << " "  << v.z << " " << v.w << " " <<  std::endl;
     }
  */
  
  /*
  for(int i = 0; i < lpd->indxNum; i += 4){
    std::cout << lpd->indx[i] << " " << lpd->indx[i+1] << " " << lpd->indx[i+2] << " " << lpd->indx[i+3] << std::endl;
    }*/

  GLuint uniforms[3];
  uniforms[0] = shader->UniformLocation("proj");
  uniforms[1] = shader->UniformLocation("view");
  uniforms[2] = shader->UniformLocation("model");

  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elms);

  //Now render the object
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

  glfwSetCursorPosCallback(window, mousePosCB);
  glfwSetMouseButtonCallback(window,mouseButtonCB);
  glfwSetWindowSizeCallback(window,screenSizeCB);

  glDisable(GL_CULL_FACE);

  while(true){
    glUniformMatrix4fv(uniforms[0],1,false,glm::value_ptr(proj));
    glUniformMatrix4fv(uniforms[1],1,false,glm::value_ptr(view));
    glUniformMatrix4fv(uniforms[2],1,false,glm::value_ptr(model));

    glClear(GL_COLOR_BUFFER_BIT);
    int offset = 0;
    for(int i = 0; i < lpd->primNum; i++){
      GLuint prim = get_prim(lpd->type[i]);

      glDrawElements( prim, lpd->icnt[i], 
		      GL_UNSIGNED_INT, ((void*) 0) + offset);
      offset += lpd->icnt[i];
      GLuint error;
      if( (error = glGetError()) != GL_NO_ERROR)
	std::cout << "GLERROR: " << error << std::endl;
    }
    glfwWaitEvents();
    glfwSwapBuffers(window);

  }

  airMopOkay(mop);
  exit(0);
}
