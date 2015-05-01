#version 400 core
// TessControl

layout(vertices = 4) out;
in vec2 vPosition[];
in vec2 vUV[];
in int vLOD[];
out vec2 tcPosition[];
out vec2 tcUV[];
const float TessLevelInner = 1;
const float TessLevelOuter = 1;

#define ID gl_InvocationID

void main()
{
    tcUV[ID] = vUV[ID];
    vec2 myPoint = vPosition[ID];
    tcPosition[ID] = myPoint;


    if (ID == 0) {
        int tess = (vLOD[0] + vLOD[1] + vLOD[2] + vLOD[3]) / 4;
        gl_TessLevelInner[0] = tess;
        gl_TessLevelInner[1] = tess;
        // Use minimum of the two vertices LOD for edge tess value
        // Such that adjacent patches have the same LOD on a shared edge
        // This way tess doesn't create gaps in the map
        gl_TessLevelOuter[0] = min(vLOD[0], vLOD[2]);
        gl_TessLevelOuter[1] = min(vLOD[0], vLOD[1]);
        gl_TessLevelOuter[2] = min(vLOD[1], vLOD[3]);
        gl_TessLevelOuter[3] = min(vLOD[2], vLOD[3]);
    }
}
