#version 330 core
out vec3 color;
in vec2 tc;
uniform sampler1D gradients;

uniform float lacunarity;
uniform float H;
uniform int octaves;
uniform int resolution;

float f(float t) {
	// For some reason, it doesn't work using the provided function:
	// return 6*pow(t, 6) - 15*pow(t, 4) + 10*pow(t, 3);

	// So we'll use good old cos² interpolation
	float c = cos(t*1.5303 /* PI/2 */);
	return 1-c*c;
}

int custom_hash(int p1, int p2)
{
   return (p2 & p1 % 557) + p1 * p2 * p2;
}

float noise(vec2 point) {
	ivec2 cell = ivec2(point);

	vec2 a = point - cell;
	vec2 b = point - (cell + vec2(1, 0));
	vec2 c = point - (cell + vec2(0, 1));
	vec2 d = point - (cell + vec2(1, 1));

	vec2 gs = normalize(texture(gradients, float(custom_hash(cell.x, cell.y))/resolution).rg);
	vec2 gt = normalize(texture(gradients, float(custom_hash(cell.x + 1, cell.y))/resolution).rg);
	vec2 gu = normalize(texture(gradients, float(custom_hash(cell.x, cell.y + 1))/resolution).rg);
	vec2 gv = normalize(texture(gradients, float(custom_hash(cell.x + 1, cell.y + 1))/resolution).rg);

	float s = dot(gs, a);
	float t = dot(gt, b);
	float u = dot(gu, c);
	float v = dot(gv, d);

	float st = mix(s, t, f(a.x));
	float uv = mix(u, v, f(a.x));

	return mix(st, uv, f(a.y));
}

float generateMap(vec2 point) {
	float value = 0.0;

	point = abs(point);

	for (int i = 0; i < octaves; i++) {
		value += abs(noise(point)) * pow(lacunarity, -H*i);
		point *= lacunarity;
	}
	return 1 - value;
}

vec2 computeDiff() {

	float dstep = 1.0/resolution;

	float yxm = generateMap(tc+vec2(-dstep, 0));
  float yxp = generateMap(tc+vec2(dstep, 0));
  float yzm = generateMap(tc+vec2(0, -dstep));
  float yzp = generateMap(tc+vec2(0, dstep));

  return vec2((yxm - yxp + 1)*0.5, (yzp - yzm + 1)*0.5);
}

void main() {
	// Heightmap in R channel, differences in G & B channels
  color = vec3(generateMap(tc), computeDiff());
}

