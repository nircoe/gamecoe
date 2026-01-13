#if __VERSION__ >= 330
in vec3 localPos;
out vec4 FragColor;
#else
varying vec3 localPos;
#endif

uniform vec4 color;
uniform int shapeType; // 0 == Circle, 1 == Sphere

void main()
{
    const float RADIUS = 1.0;

    float multiplier = (shapeType == 0) ? 2.1 : 1.75;
    vec3 samplePos = localPos * multiplier;
    float dist = length(samplePos);

    float edgeWidth = fwidth(dist);
    float alpha = 1.0 - smoothstep(RADIUS - edgeWidth, RADIUS + edgeWidth, dist);

#if __VERSION__ >= 330
    FragColor = vec4(color.rgb, color.a * alpha);
#else
    gl_FragColor = vec4(color.rgb, color.a * alpha);
#endif
}
