#version 130

in vec4 color_frag;
out vec4 fcol;

void main(void) {

  // fcol = vec4(1.0,0,0,1.0);
  fcol = color_frag;
}
