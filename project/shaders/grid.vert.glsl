#version 330 core

uniform mat4 mvp;
uniform sampler2D tex;

in vec2 position;
out vec2 uv;

void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;

    float height = texture(tex, uv).r;
    vec3 pos_3d = vec3(position.x, height, -position.y);

    gl_Position = mvp * vec4(pos_3d, 1.0);
}
