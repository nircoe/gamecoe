uniform vec4 color;

#if __VERSION__ >= 330
out vec4 FragColor;
#endif

void main()
{
#if __VERSION__ >= 330
    FragColor = color;
#else
    gl_FragColor = color;
#endif
}
