#version 330 core
layout (location = 0) in vec3 aPos;
// texture, color, etc, in the future

uniform mat4 model;
// add those uniforms when Camera is functional:
// uniform mat4 view;
// uniform mat4 projection;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
    // gl_Position = projection * view * model * vec4(aPos, 1.0);
}
