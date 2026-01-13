#if __VERSION__ >= 330
in vec3 localPos;
#else
varying vec3 localPos;
#endif

#if __VERSION__ >= 330
out vec4 FragColor;
#endif

uniform vec4 color;

void main()
{
    const float RADIUS = 1.0;
    const float SMOOTHNESS = 0.015;

    vec3 samplePos = localPos * 2.1;
    float dist = length(samplePos);

    float alpha = 1.0 - smoothstep(RADIUS - SMOOTHNESS, RADIUS + SMOOTHNESS, dist);

#if __VERSION__ >= 330
    FragColor = vec4(color.rgb, color.a * alpha);
#else
    gl_FragColor = vec4(color.rgb, color.a * alpha);
#endif
}
