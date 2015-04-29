#version 330 core
out vec3 color;
in vec2 uv;
uniform sampler2D tex_through;
uniform sampler2D tex_mirror;
uniform float time;

float freqencies[4] = float[](130, 1103, 5503, 9907);

vec2 distort(float freq) {
    return vec2(cos(time+uv.x*freq), sin(time+uv.y*freq))/1000;
}

void main() {
    /// query window_width/height using the textureSize(..) function on tex_mirror
    ivec2 dim = textureSize(tex_mirror, 0);

    // This is just a test for what could be the wave generator
    vec2 distortion = vec2(0, 0);
    for (int i = 0 ; i < 4 ; ++i) {
        distortion += distort(freqencies[i]);
    }

    float dx = dim.x;
    float dy = dim.y;
    /// use gl_FragCoord to build a new [_u,_v] coordinate to query the framebuffer
    /// make sure you normalize gl_FragCoord by window_width/height
    float u = gl_FragCoord.x / dx;
    float v = gl_FragCoord.y / dy;


    // Mixing and distorting. No reflection has gone into this "+ distortion" thing. It might actually completely wrong.
    // TODO: investigate to see if distortion on reflection matches distortion on refraction in a realistic way
    color = mix( texture(tex_through, vec2(u,v) + distortion).rgb, texture(tex_mirror, vec2(u,1-v) + distortion).rgb, .6);
}
