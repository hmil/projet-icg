#version 330 core


in vec2 position;
out vec2 tc;

void main() {
    tc = (position + vec2(1.0, 1.0)) * 0.5;

    gl_Position = vec4(position, 0.0, 1.0);
}
