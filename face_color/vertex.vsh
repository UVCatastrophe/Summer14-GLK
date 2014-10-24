#version 150 core

in vec3 position;
out vec3 vcol;
out vec2 tex_coord;

void main(void) { 
    gl_Position = vec4(position, 1.0);
    tex_coord = (position.xy + 1.0)/2.0;
    tex_coord.y = 1 - tex_coord.y;
}
