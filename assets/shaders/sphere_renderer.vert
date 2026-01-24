#if __VERSION__ >= 330
layout (location = 0) in vec3 aPos;
out vec3 localPos;
out vec3 localCamPos;
#else
attribute vec3 aPos;
varying vec3 localPos;
varying vec3 localCamPos;
#endif

uniform mat4 model;
uniform mat4 inverseModel;
#if GAMECOE_HAS_UBO
layout(std140) uniform CameraMatrices
{
    mat4 projection;
    mat4 view;
    vec3 cameraPosition;
};
#else
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;
#endif

void main()
{
    localPos = aPos;
    localCamPos = vec3(inverseModel * vec4(cameraPosition, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
