#version 330 core

uniform mat4 mvp;
in vec3 position;
out vec3 texcoords;

void main() {
    vec4 pos = mvp * vec4(position, 1.0);
    //pos.y = -1*pos.y;
    gl_Position = pos;

	vec3 pos2 = position;
	texcoords = pos2;
}
