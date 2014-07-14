#include "shader.h"
#include <sys/stat.h>
#include <cstring>
#include <GLFW/glfw3.h>
#include <iostream>

ShaderProgram::ShaderProgram(){
  this->progId = glCreateProgram();
  this->vshId = glCreateShader(GL_VERTEX_SHADER);
  this->fshId = glCreateShader(GL_FRAGMENT_SHADER);
}

static GLchar *ReadFile (const char *file)
{
    struct stat	st;

  /* get the size of the file */
    if (stat(file, &st) < 0) {
	std::cerr << "error reading \"" << file << "\": " << strerror(errno) << std::endl;
	return 0;
    }
    off_t sz = st.st_size;

  /* open the file */
    FILE *strm = fopen(file, "r");
    if (strm == NULL) {
	std::cerr << "error opening \"" << file << "\": " << strerror(errno) << std::endl;
	return 0;
    }

  /* allocate the buffer */
    GLchar *buf = new GLchar[sz+1];

  /* read the file */
    if (fread(buf, 1, sz, strm) != size_t(sz)) {
	std::cerr << "error reading \"" << file << "\": " << strerror(errno) << std::endl;
	free (buf);
    }
    buf[sz] = '\0';

    fclose (strm);

    return buf;

}

//! \brief load and compile a shader from a file
//! \param file the source file name
//! \return true if successful and fals if there was an error
//
void ShaderProgram::shader_LoadFromFile (const char *file, int kind,int id)
{
    GLint sts;

    GLchar *src = ReadFile (file);

    if (src == nullptr)
	return;

    glShaderSource (id, 1, &src, 0);
    delete src;

    glCompileShader (id);

  /* check for errors */
    glGetShaderiv (id, GL_COMPILE_STATUS, &sts);
    if (sts != GL_TRUE) {
      /* the compile failed, so report an error */
	glGetShaderiv (id, GL_INFO_LOG_LENGTH, &sts);
	if (sts != 0) {
	    GLchar *log =  new GLchar[sts];
	    glGetShaderInfoLog (id
, sts, 0, log);
	    std::cerr << "Error compiling " << " shader \""
		<< file << "\":\n" << log << "\n" << std::endl;
	    delete log;
	}
	else
	    std::cerr << "Error compiling " <<  " shader \""
		<< file << "\":\n  no log\n" << std::endl;
	return;
    }

    return;

}

bool ShaderProgram::vertexShader(const char* file){
  this->shader_LoadFromFile(file, GL_VERTEX_SHADER, this->vshId);
  glAttachShader(this->progId, this->vshId);
  return true;
}

bool ShaderProgram::fragmentShader(const char* file){
  this->shader_LoadFromFile(file,GL_FRAGMENT_SHADER,this->fshId);
  glAttachShader(this->progId, this->fshId);
  return true;
}

GLint ShaderProgram::UniformLocation (const char *name)
{
    GLint id = glGetUniformLocation (this->progId, name);
    if (id < 0) {
	std::cerr << "Error: shader uniform \"" << name << "\" is invalid\n" << std::endl;
	exit (1);
    }
    return id;
}
