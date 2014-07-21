#version 130

uniform mat4 proj;
uniform mat4 view;

in vec4 position;
in vec3 norm;
in vec4 color;
out vec4 color_frag;

void main(void) { 

    gl_Position = proj * view * position;
    color_frag = color;
    
}
