#version 400 core

#define M_PI 3.14159265359

out vec4 FragColor;

in vec3 gFacetNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;
in vec2 gUV;
in vec3 gNormal;

uniform vec3 Ia, Id;
uniform sampler2D tex;
uniform sampler2D grass_tex;
uniform sampler2D rock_tex;
uniform sampler2D sand_tex;
uniform sampler2D snow_tex;
uniform vec3 light_dir;
uniform float cam_height;


const float SNOW_INTERP_FACTOR = 100;
const float GRASS_INTERP_FACTOR = 100;
const float SAND_INTERP_FACTOR = 200;
const float ROCK_INTERP_FACTOR = 40;

const float GRASS_MIN_HEIGHT = 0.3; // Also water level
const float SNOW_MIN_HEIGHT = 0.65;
const float SNOW_VARIANCE = 0.09;

const float SLOPE_THRESHOLD = 0.25;

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

vec4 classicShading() {
  // Sample height data once
  vec3 sampled = texture(tex, gUV).rgb;
  float height = sampled.r;
  float slope = abs(sampled.b - 0.5) + abs(sampled.g - 0.5);

  // Texture distortion to snow level for realism
  float snow_spatial_dist = sin((gUV.x+gUV.y)*20*M_PI + slope);
  float snow_slope_dist = (slope * slope * 10);
  float snowActualHeight = SNOW_MIN_HEIGHT + SNOW_VARIANCE * snow_spatial_dist * snow_slope_dist;

  // At higher altitudes, rock is more likely to appear on flatter slopes
  float slopeActualThreshold = (1 - 0.2*height) * SLOPE_THRESHOLD;

  // Texturing
  float grassAmount = 0, rockAmount = 0, sandAmount = 0, snowAmount = 0;
  rockAmount = clamp(ROCK_INTERP_FACTOR*(slope - slopeActualThreshold), 0, 1);
  snowAmount = clamp(SNOW_INTERP_FACTOR*(height-snowActualHeight), 0, 1);
  grassAmount = clamp(GRASS_INTERP_FACTOR*(height-GRASS_MIN_HEIGHT), 0, 1);
  sandAmount = 1 - grassAmount;
  rockAmount = clamp(rockAmount - sandAmount, 0, 1); // There's no rock underwater
  // In case rock is at snow level, we "freeze" it with some snow:
  rockAmount = clamp(rockAmount - clamp(height - snowActualHeight, 0, 1), 0, 1);
  // no snow on rock
  snowAmount = clamp(snowAmount - rockAmount, 0, 1);
  grassAmount = clamp(grassAmount - snowAmount - rockAmount, 0, 1); // There's no grass on rock

  // Computing base color from textures
  vec3 baseColor =
            grassAmount* texture(grass_tex, gUV * 100).rgb
          + rockAmount * texture(rock_tex , gUV * 100).rgb
          + sandAmount * texture(sand_tex , gUV * 60 ).rgb
          + snowAmount * texture(snow_tex , gUV * 30 ).rgb;

  // Apply shading
  vec3 finalColor = Ia*baseColor + clamp(Id*baseColor*dot(normalize(gNormal), normalize(light_dir)), vec3(0,0,0), vec3(1,1,1));

  // Underwater effect
  float water_coeff = 0;
  if(height < GRASS_MIN_HEIGHT) {
    water_coeff = sqrt(GRASS_MIN_HEIGHT - height);
    finalColor = mix(finalColor, vec3(0, 0, 0.2), water_coeff);
  }

  float disperse_distance = GRASS_MIN_HEIGHT - height;
  if (cam_height < 0) {
    disperse_distance = getDepth();
  }
  finalColor = disperseUnderwater(finalColor, disperse_distance);

  // return vec4(finalColor, 0.7); // set this to help with curves design
  return vec4(finalColor, 1);
}

// Utility function for tessellation debug drawing
float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

vec4 tessellationDebugShading() {
  vec3 N = normalize(gFacetNormal);
  vec3 L = vec3(10, 10, 10);
  float df = abs(dot(N, L));
  vec3 color = vec3(0.2, 0.05, 0.05) + df * vec3(0.1, 0.1, 1);

  float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
  float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
  color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;

  return vec4(color, 1.0);
}

void main()
{
  FragColor = classicShading();
  //FragColor = tessellationDebugShading();
}
