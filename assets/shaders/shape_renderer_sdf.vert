#if __VERSION__ >= 330
layout (location = 0) in vec3 aPos;
#else
attribute vec3 aPos;
#endif

#if __VERSION__ >= 330
out vec3 localPos;
#else
varying vec3 localPos;
#endif

uniform mat4 model;
#if GAMECOE_HAS_UBO
layout(std140) uniform CameraMatrices
{
    mat4 projection;
    mat4 view;
};
#else
uniform mat4 view;
uniform mat4 projection;
#endif

void main()
{
    localPos = aPos;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
