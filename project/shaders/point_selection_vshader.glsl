#version 330 core

uniform mat4 mvp;
uniform vec3 pos;

void main(){
    gl_Position = mvp * vec4(pos, 1.0);
    gl_PointSize = 10.0;// / gl_Position.z;
}
