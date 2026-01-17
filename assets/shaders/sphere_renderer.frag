#if __VERSION__ >= 330
in vec3 localPos;
in vec3 localCamPos;
out vec4 FragColor;
#else
varying vec3 localPos;
varying vec3 localCamPos;
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

vec4 sphereHit(out vec3 normal)
{
    vec3 rayDirection = normalize(localPos - localCamPos);

    // float a = dot(rayDirection, rayDirection) = 1.0;
    float b = 2.0 * dot(localCamPos, rayDirection);
    float c = dot(localCamPos, localCamPos) - 1.0;

    float discriminant = b * b - 4.0 * c;

    if (discriminant < 0.0) discard;

    float sqrtDisc = sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / 2.0;
    float t2 = (-b + sqrtDisc) / 2.0;

    float t;
    if (t1 > 0.0) t = t1;
    else if (t2 > 0.0) t = t2;
    else discard;

    vec3 localHit = localCamPos + t * rayDirection;
    vec4 worldHit = model * vec4(localHit, 1.0);

    vec3 localNormal = normalize(localHit);
    if (t1 <= 0.0) localNormal = -localNormal;

    vec3 worldNormal = normalize(mat3(model) * localNormal);
    normal = worldNormal;

    return worldHit;
}

void main()
{
    vec3 normal = vec3(0.0);
    vec4 spherePoint = sphereHit(normal);

    vec4 clipPos = projection * view * spherePoint;
    gl_FragDepth = (clipPos.z / clipPos.w) * 0.5 + 0.5; // how should I version gate this?

    float diffuse = max(dot(normal, normalize(vec3(1.0, 2.0, 3.0))), 0.55);

#if __VERSION__ >= 330
    FragColor = vec4(color.rgb * diffuse, color.a);
#else
    gl_FragColor = vec4(color.rgb * diffuse, color.a);
#endif
}