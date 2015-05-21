#version 330 core

uniform mat4 mvp;
in vec3 position;
out vec3 texcoords;

void main() {
    vec4 glpos = mvp * vec4(vec3(position.x, -position.y, position.z), 1.0);
	//glpos.y = -glpos.y;
	gl_Position = glpos;

	texcoords = position;
}
