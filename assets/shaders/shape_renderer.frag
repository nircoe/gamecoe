#if __VERSION__ >= 330
out vec4 FragColor;
#endif

uniform vec4 color;

void main()
{
#if __VERSION__ >= 330
    FragColor = color;
#else
    gl_FragColor = color;
#endif
}
