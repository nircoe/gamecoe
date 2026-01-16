#if __VERSION__ >= 330
in vec3 worldPos;
out vec4 FragColor;
#else
varying vec3 worldPos;
#endif

uniform mat4 model;
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
uniform vec3 sphereCenter;
uniform vec3 sphereScale;
uniform mat3 sphereRotation;

vec3 sphereIntersection(vec3 rayOrigin, vec3 wPos)
{
    const float RADIUS = 1.0;
    const float BOX_RADIUS = 0.5;

    mat3 sphereRotationT = transpose(sphereRotation);

    vec3 effectiveScale = sphereScale * BOX_RADIUS;
    vec3 rayDirection = vec3(sphereRotationT * normalize(wPos - rayOrigin)) / effectiveScale;
    vec3 sphereCenterToRayOrigin = (sphereRotationT * vec3(rayOrigin - sphereCenter)) / effectiveScale;

    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(sphereCenterToRayOrigin, rayDirection);
    float c = dot(sphereCenterToRayOrigin, sphereCenterToRayOrigin) - (RADIUS * RADIUS);

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
    {
        return vec3(0.0);
    }

    float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    float t2 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t = t2 > 0.0 ? t2 : t1;
    vec3 intersection = rayOrigin + (sphereRotation * (vec3(t * rayDirection) * effectiveScale));

    return intersection;
}

void main()
{
    vec3 spherePoint = sphereIntersection(cameraPosition, worldPos);
    if (spherePoint == vec3(0.0))
    {
        discard;
    }

    vec4 shadingPos = projection * view * model * vec4(spherePoint, 1.0);
    shadingPos /= shadingPos.w;
    gl_FragDepth = shadingPos.z; // need version gating?

#if __VERSION__ >= 330
    FragColor = color;
#else
    gl_FragColor = color;
#endif
}