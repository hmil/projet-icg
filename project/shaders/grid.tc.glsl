#version 400 core
// TessControl

layout(vertices = 3) out;
in vec2 vPosition[];
in vec2 vUV[];
out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
} vert[];
out vec2 tcPosition[];
out vec2 tcUV[];
const float TessLevelInner = 1;
const float TessLevelOuter = 1;

uniform mat4 MVP;
uniform mat4 model;

#define ID gl_InvocationID

void main()
{
    tcUV[ID] = vUV[ID];
    vec2 myPoint = vPosition[ID];
    tcPosition[ID] = myPoint;

    vert[ID].gl_ClipDistance[0] = -1;

    vec4 pp = model * vec4(myPoint.x, 0, -myPoint.y, 1.0 );
    pp /= pp.w;

    int tess = int((1 - clamp(length(vec2(pp.x, pp.z)) / 5 - 0.5, 0, 1)) * 6);

    if (ID == 0) {
        gl_TessLevelInner[0] = tess;
        gl_TessLevelOuter[0] = 3;
        gl_TessLevelOuter[1] = 3;
        gl_TessLevelOuter[2] = 3;
    }
}
