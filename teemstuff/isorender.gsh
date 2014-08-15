#version 150

uniform vec3 origin;
uniform vec3 ray;
uniform int pixel_trace;

layout(triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VertexData{
     vec3 norm;
     vec4 color;
}VertexIn[3];

out VertexData {
     vec3 norm;
     vec4 color;
}VertexOut;


void main(){
    float a,f,u,v;

}
