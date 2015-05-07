#version 330 core
in vec2 vpoint;
in vec2 vtexcoord;
out vec2 uv;
out vec2 vpos;

void main() {
    gl_Position = vec4(vpoint, 0, 1.0);
    vpos = vpoint;
    uv = vtexcoord;
}
