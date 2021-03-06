#version 330 core
out vec4 color;
in vec2 uv;
in vec4 view_dir;
in vec2 world_pos;
uniform sampler2D tex_through;
uniform sampler2D tex_mirror;
uniform float time;

uniform vec2 cam_pos;

const float M_PI = 3.14159265358979323846;

const float zNear = 0.01;
const float zFar = 10.0;
float getDepth() {
  float z_b = gl_FragCoord.z;
  float z_n = 2.0 * z_b - 1.0;
  return clamp(2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear)), 0, 1);
}
vec3 disperseUnderwater(vec3 colorIn, float amount) {
  return mix(colorIn, vec3(0, 0, 0.2), amount);
}


#define harmonics 9
float freqencies[harmonics] = float[](130, 3,  113, 550,  907,   63, 29, 439, 713 );
float angles[harmonics] = float[](0.1234, 0.1, 1.32, 0.4, 1.89, 0.87, 0.23, 0.97, 0 );
vec2 distort(float freq, float angle) {
    vec2 value = -time*sqrt(freq)/20 + (world_pos + cam_pos) * freq;

    mat2 rot = mat2(cos(angle), sin(angle), -sin(angle), cos(angle));

    value = value * rot;
    return vec2(cos(value.x * M_PI), sin(value.y * M_PI))/1000;
}
void main() {
    /// query window_width/height using the textureSize(..) function on tex_mirror
    ivec2 dim = textureSize(tex_mirror, 0);

    // This is just a test for what could be the wave generator
    vec2 distortion = vec2(0, 0);
    for (int i = 0 ; i < harmonics ; ++i) {
        distortion += distort(freqencies[i], angles[i]);
    }

    float dx = dim.x;
    float dy = dim.y;
    /// use gl_FragCoord to build a new [_u,_v] coordinate to query the framebuffer
    /// make sure you normalize gl_FragCoord by window_width/height
    float u = gl_FragCoord.x / dx;
    float v = gl_FragCoord.y / dy;

    float view_dir_coeff = dot(normalize(view_dir), vec4(0, 1, 0, 0));
    view_dir_coeff = abs(view_dir_coeff);
    //view_dir_coeff = 1.0 - view_dir_coeff;
    // Mixing and distorting. No reflection has gone into this "+ distortion" thing. It might actually completely wrong.
    // TODO: investigate to see if distortion on reflection matches distortion on refraction in a realistic way

    float mix_coeff = clamp(1 - view_dir_coeff, 0, 1);
    vec3 color_tmp = mix( texture(tex_through, vec2(u,v) + distortion).rgb, texture(tex_mirror, vec2(u,1-v) + distortion).rgb, mix_coeff);

    // underwater loss of light
    if (view_dir.y < 0) {
        color_tmp = disperseUnderwater(color_tmp, getDepth());
    }

    color = vec4(color_tmp, 1);
    // color = vec4(color_tmp, 0.3); // activate to debug curves
    //color = vec3(distortion.x * 100, distortion.y * 100, 0);
}
