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

vec3 sphereIntersection(vec3 rayOrigin, vec3 wPos, out vec3 normal)
{
    const float RADIUS = 1.0;

    mat3 sphereRotationT = transpose(sphereRotation);

    vec3 rayDirection = vec3(sphereRotationT * normalize(wPos - rayOrigin)) / sphereScale;
    vec3 sphereCenterToRayOrigin = (sphereRotationT * vec3(rayOrigin - sphereCenter)) / sphereScale;

    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(sphereCenterToRayOrigin, rayDirection);
    float c = dot(sphereCenterToRayOrigin, sphereCenterToRayOrigin) - (RADIUS * RADIUS);

    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) return vec3(0.0);

    float sqrtDisc = sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0 * a);
    float t2 = (-b + sqrtDisc) / (2.0 * a);

    if (t1 < 0.0 && t2 < 0.0) return vec3(0.0);
    
    float t;
    if (t1 > 0.0) t = t1;
    else if (t2 > 0.0) t = t2;
    else return vec3(0.0);

    vec3 intersection = rayOrigin + (sphereRotation * (t * rayDirection * sphereScale));
    vec3 localIntersection = (sphereRotationT * (intersection - sphereCenter)) / sphereScale;
    normal = sphereRotation * localIntersection;
    
    if (t1 <= 0.0) normal = -normal;

    return intersection;
}

void main()
{
    vec3 normal = vec3(0.0);
    vec3 spherePoint = sphereIntersection(cameraPosition, worldPos, normal);
    if (spherePoint == vec3(0.0)) discard;

    vec4 shadingPos = projection * view * vec4(spherePoint, 1.0);
    gl_FragDepth = (shadingPos.z / shadingPos.w) * 0.5 + 0.5;

    float diffuse = max(dot(normalize(vec3(1.0, 2.0, 3.0)), normal), 0.55);

#if __VERSION__ >= 330
    FragColor = vec4(color.rgb * diffuse, color.a);
#else
    gl_FragColor = vec4(color.rgb * diffuse, color.a);
#endif
}