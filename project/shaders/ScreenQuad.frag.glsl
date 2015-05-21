#version 330 core
uniform sampler2D color_tex;
uniform sampler2D depth_tex;
uniform sampler2D clouds_tex;

uniform float tex_width;
uniform float tex_height;

uniform mat4 VP_i;
uniform vec3 cam_pos;
uniform vec3 light_dir = normalize(vec3(-1.0, 1.0, -1.0));
uniform float time;

in vec2 uv;
in vec2 vpos;
out vec3 color;

#define M_PI 3.1415

const float CLOUD_DENSITY = 10;
const float CLOUD_FLOOR = 0.6;
const float CLOUD_AMPLITUDE = 4; // determines cloud top height
const float CLOUD_CUTOFF = 0.75;
// Nyquist stuff going on here: larger clouds allow smaller sampling frequency
// and are therefore less expensive to render.
// If it laggs: turn sampling down. If it flickers: turn scale up.
const int CLOUD_SCALE = 30;
const int CLOUD_SAMPLING = 70;

float rgb_2_luma(vec3 c){ return .3*c[0] + .59*c[1] + .11*c[2]; }

vec2 makeCloudTexCoord(vec2 uv) {
  return abs(mod(vec2(uv.x + time/20, uv.y), CLOUD_SCALE)) * 2 / CLOUD_SCALE - 1;
}

float getCloudDensity(vec3 p) {
  //return sin(p.x*2)*max(0, sin(min(p.y - CLOUD_FLOOR, 2)*M_PI/2))*sin(p.z*2);

  vec2 tc = makeCloudTexCoord(vec2(p.x, p.z));
  float noiseval = max(0, (CLOUD_CUTOFF - texture(clouds_tex, tc).r));
  float freq = M_PI / (noiseval*CLOUD_AMPLITUDE);

  float cFloor = CLOUD_FLOOR - noiseval*CLOUD_AMPLITUDE/5;

  if (p.y < cFloor) return 0;

  // make a symetric cloud wrt xz-plane
  return sin(min(M_PI, (p.y - cFloor) * freq)) * noiseval;
  //return noiseval*max(0, sin(min(p.y - CLOUD_FLOOR, 2)*M_PI/2)) - noiseval * p.y;
}

float f(float depth) {
  return sqrt(depth);
}

vec3 unproject(float depth) {
  vec4 vect = VP_i * vec4(vpos, depth, 1.0);
  return vec3(vect) / vect.w;
}

void main() {
  vec3 current_color = texture(color_tex, uv).rgb;
  float current_depth = texture(depth_tex, uv).r;

  vec2 vertex_pos = vpos;
  vertex_pos.y = -vertex_pos.y; // Inverse y coordinate, figure out why after

  vec3 nearPoint = unproject(0);
  vec3 farPoint = unproject(1);
  vec3 maxPoint = unproject(current_depth);
  float horizon_distance = distance(farPoint, nearPoint);
  float step = horizon_distance / CLOUD_SAMPLING;
  vec3 direction = (farPoint - nearPoint) / horizon_distance;

  vec3 view_dir = normalize(direction);

  float cutoff_value = min(distance(nearPoint, maxPoint), horizon_distance);

  // sun
  float vl = dot(view_dir, light_dir);
  if (current_depth == 1 && vl > 0) {
    current_color = min(vec3(pow(vl, 1000)) + current_color, vec3(1, 1, 1));
  }

  // Poor man's shadow map
  if (current_depth != 1) {
    // Take the world point corresponding to that pixel
    vec3 shadowPoint = maxPoint + vec3(cam_pos.x, 0, cam_pos.z);

    // sample the clouds above it
    shadowPoint.y = CLOUD_FLOOR;
    float d = getCloudDensity(shadowPoint);

    // And shade accordingly
    current_color = mix(current_color, vec3(0, 0, 0), d*3);
  }

  float iteration_value;
  for (iteration_value = 0 ; iteration_value < cutoff_value ; iteration_value += step) { }
  iteration_value -= step;

  for ( ; iteration_value > 0.1 ; iteration_value -= step) {

    vec3 world_point = nearPoint + direction * iteration_value + vec3(cam_pos.x, 0, cam_pos.z);
    //
    float coeff = min(iteration_value + step, cutoff_value) - iteration_value;


    float density = getCloudDensity(world_point);

    vec3 cloudColor = mix(vec3(0.50, 0.55, 0.59), vec3(0.85, 0.90, 0.95), clamp((world_point.y - CLOUD_FLOOR)*3*density*CLOUD_AMPLITUDE + 0.5, 0, 1));

    current_color = mix(current_color, cloudColor, min(max(density, 0)*coeff*CLOUD_DENSITY, 1));
  }

  color = current_color;
  //color = vec3(1 - texture(clouds_tex, uv).r, 0, 0);

  /*vec4 world_point = VP_i * vec4(vpos, 0.9, 1);
  world_point /= world_point.w;

  color = vec3(world_point.x, world_point.y, world_point.z);*/
}

