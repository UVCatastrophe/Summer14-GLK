
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


char *pdInfo=("Program to demo limnPolyData (as well as the "
              "hest command-line option parser)");


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
  glBufferData(GL_ARRAY_BUFFER, lpd->xyzwNum*sizeof(float),
	       lpd->xyzw, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT,GL_FALSE,0, 0);

  //Norms
  glBindBuffer(GL_ARRAY_BUFFER, bufs[1]);
  glBufferData(GL_ARRAY_BUFFER, lpd->normNum*sizeof(float),
	       lpd->norm, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT,GL_FALSE,0, 0);

  //Colors
  glBindBuffer(GL_ARRAY_BUFFER, bufs[2]);
  glBufferData(GL_ARRAY_BUFFER, lpd->rgbaNum*sizeof(char),
	       lpd->rgba, GL_STATIC_DRAW);
  glVertexAttribPointer(2, 4, GL_BYTE,GL_FALSE,0, 0);



  //Indices
  GLuint elms;
  glGenBuffers(1, &elms);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elms);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
	       lpd->indxNum * sizeof(int),
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

  //Set up the view/projection matrix
  glm::mat4 view = glm::lookAt(glm::vec3(5.0f,5.0f,5.0f),
			       glm::vec3(0.0f,0.0f,0.0f),
			       glm::vec3(0.0,1.0,0.0));

  glm::mat4 proj = glm::perspective(.73f, 640.0f/480.0f,.1f, 50.0f);
  /*
  for(int i = 0; i < lpd->xyzwNum; i += 4){
    float x = lpd->xyzw[i];
    float y = lpd->xyzw[i+1];
    float z = lpd->xyzw[i+2];
    float w = lpd->xyzw[i+3];
    glm::vec4 v = glm::vec4(x,y,z,w);
    v = (proj * view * v);
    std::cout << v.x << " " << v.y << " "  << v.z << " " << v.w << " " <<  std::endl;
  }*/

  GLuint uniforms[2];
  uniforms[0] = shader->UniformLocation("proj");
  uniforms[1] = shader->UniformLocation("view");

  glUniformMatrix4fv(uniforms[0],1,false,glm::value_ptr(proj));
  glUniformMatrix4fv(uniforms[1],1,false,glm::value_ptr(view));


  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elms);

  //Now render the object
  while(true){

    glDrawElements( GL_TRIANGLE_STRIP, lpd->indxNum, 
		    GL_UNSIGNED_INT, (GLvoid*)0);
    if(glGetError() != GL_NO_ERROR)
      std::cout << "fail\n";
    glfwSwapBuffers(window);

  }

  airMopOkay(mop);
  exit(0);
}
