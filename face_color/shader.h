#ifndef _SHADER_H_
#define _SHADER_H_

#define GLFW_INCLUDE_GLU
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

class ShaderProgram{
 public:
  GLuint progId;
  GLuint vshId;
  GLuint fshId;

  bool vertexShader(const char* file);
  GLint UniformLocation (const char *name);
  bool fragmentShader(const char* file);

  ShaderProgram (); 

 protected:

  void shader_LoadFromFile (const char *file, int kind,int id);

};

#endif
