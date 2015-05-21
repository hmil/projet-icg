#version 330 core
in vec3 texcoords;
out vec3 color;

uniform samplerCube tex;

void main() {
    color = texture(tex, texcoords).rgb;
}
