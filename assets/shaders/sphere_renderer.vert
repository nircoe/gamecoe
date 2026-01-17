#if __VERSION__ >= 330
layout (location = 0) in vec3 aPos;
out vec3 worldPos;
#else
attribute vec3 aPos;
varying vec3 worldPos;
#endif

uniform mat4 model;
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
    const float SCALE_MULTIPLIER = 2; // Quad is -0.5 to 0.5, scale to radius 1.0

    vec3 scaledPos = aPos * SCALE_MULTIPLIER;
    vec4 wPos = model * vec4(scaledPos, 1.0);
    worldPos = vec3(wPos);
    gl_Position = projection * view * wPos;
}
