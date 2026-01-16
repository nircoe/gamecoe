#if __VERSION__ >= 330
in vec3 localPos;
out vec4 FragColor;
#else
varying vec3 localPos;
#endif

uniform vec4 color;

float circleAlpha(vec3 lPos)
{
    const float RADIUS = 1.0;
    const float MULTIPLIER = 2.1; // Quad is -0.5 to 0.5, scale to radius 1.0

    vec3 samplePos = lPos * MULTIPLIER;

    float dist = length(samplePos);
    float edgeWidth = fwidth(dist);
    float alpha = 1.0 - smoothstep(RADIUS - edgeWidth, RADIUS + edgeWidth, dist);

    return alpha;
}

void main()
{
    float alpha = circleAlpha(localPos);

#if __VERSION__ >= 330
    FragColor = vec4(color.rgb, color.a * alpha);
#else
    gl_FragColor = vec4(color.rgb, color.a * alpha);
#endif
}
