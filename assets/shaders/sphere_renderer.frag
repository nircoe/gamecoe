#if __VERSION__ >= 330
in vec3 localPos;
out vec4 FragColor;
#else
varying vec3 localPos;
#endif

uniform vec4 color;
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

float sphereAlpha(vec3 rayOrigin, vec3 lPos)
{
    // Sphere center at (0, 0, 0)
    const float RADIUS = 1.0;
    const float MULTIPLIER = 1.75; // Box is -0.5 to 0.5, scale to radius 1.0

    vec3 samplePos = lPos * MULTIPLIER;
    vec3 rayDirection = vec3(normalize(samplePos - rayOrigin));

    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(rayOrigin, rayDirection);
    float c = dot(rayOrigin, rayOrigin) - (RADIUS * RADIUS);

    float discriminant = b * b - 4.0 * a * c;

    return (discriminant >= 0.0) ? 1.0 : 0.0;
    /*
    if (discriminant < 0.0)
    {
        return 0.0;
    }

    float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    float t2 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t = t2 > 0.0 ? t2 : t1;
    vec3 intersection = rayOrigin + vec3(t * rayDirection);
    */
}

void main()
{
    float alpha = sphereAlpha(cameraPosition, localPos);

#if __VERSION__ >= 330
    FragColor = vec4(color.rgb, color.a * alpha);
#else
    gl_FragColor = vec4(color.rgb, color.a * alpha);
#endif
}