layout (location = 0) in vec3 aPos;
// texture, etc... in the future

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
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
