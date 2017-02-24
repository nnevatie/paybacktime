#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texLight;
uniform mat4      v;
uniform mat4      p;
uniform mat4      pc;
uniform vec3      viewPos;
uniform float     zNear;
uniform float     zFar;
uniform float     scale;

// Const
vec2 sizeTex = textureSize(texDepth, 0);
vec4 up      = v * vec4(0, 1, 0, 0);

#define float4 vec4
#define float3 vec3
#define float2 vec2

int   _Iterations          = 20;
float _MaxRayDistance      = 50.0;
float _PixelStride         = 20.0;
float _PixelStrideZCuttoff = 500.0;
float _PixelThickness      = 5.0;
vec3  _ProjectionParams    = vec3(0, zNear, zFar);
float _ScreenEdgeFadeStart = 0.75;
float _EyeFadeStart        = 0.0;
float _EyeFadeEnd          = 1.0;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

void swap(inout float f0, inout float f1)
{
    float t = f0;
    f0 = f1;
    f1 = t;
}

float distanceSquared(vec2 a, vec2 b)
{
    a -= b;
    return dot(a, a);
}

bool intersectDepth(vec2 uv, float zMin, float zMax)
{
    float cameraZ = texture(texDepth, uv).r;
    return -zMax >= cameraZ && -zMax <= cameraZ + _PixelThickness;
}

bool raytrace(float3 rayOrigin,
              float3 rayDirection,
              float jitter,
              out float2 hitPixel,
              out float3 hitPoint,
              out float iterationCount)
{
    // Clip to the near plane
    float rayLength = ((rayOrigin.z + rayDirection.z * _MaxRayDistance) > -_ProjectionParams.y) ?
                       (-_ProjectionParams.y - rayOrigin.z) / rayDirection.z : _MaxRayDistance;
    float3 rayEnd = rayOrigin + rayDirection * rayLength;

    // Project into homogeneous clip space
    float4 H0 = pc * float4(rayOrigin, 1.0);
    float4 H1 = pc * float4(rayEnd,    1.0);

    float k0 = 1.0 / H0.w,
          k1 = 1.0 / H1.w;

    // The interpolated homogeneous version of the camera-space points
    float3 Q0 = rayOrigin * k0,
           Q1 = rayEnd    * k1;

    // Screen-space endpoints
    float2 P0 = H0.xy * k0,
           P1 = H1.xy * k1;

    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += (distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0;

    float2 delta = P1 - P0;

    // Permute so that the primary iteration is in x to collapse
    // all quadrant-specific DDA cases later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y))
    {
        // This is a more-vertical line
        permute = true; delta = delta.yx; P0 = P0.yx; P1 = P1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    float3  dQ = (Q1 - Q0) * invdx;
    float dk   = (k1 - k0) * invdx;
    float2  dP = float2(stepDir, delta.y * invdx);

    // Calculate pixel stride based on distance of ray origin from camera.
    // Since perspective means distant objects will be smaller in screen space
    // we can use this to have higher quality reflections for far away objects
    // while still using a large pixel stride for near objects (and increase performance)
    // this also helps mitigate artifacts on distant reflections when we use a large
    // pixel stride.
    float strideScaler = 1.0 - min(1.0, -rayOrigin.z / _PixelStrideZCuttoff);
    float pixelStride = 1.0 + strideScaler * _PixelStride;

    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dP *= pixelStride; dQ *= pixelStride; dk *= pixelStride;
    P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;

    float i, zA = 0.0, zB = 0.0;

    // Track ray step and derivatives in a float4 to parallelize
    float4 pqk = float4( P0, Q0.z, k0);
    float4 dPQK = float4( dP, dQ.z, dk);
    bool intersect = false;

    for( i=0; i<_Iterations && intersect == false; i++)
    {
        pqk += dPQK;

        zA = zB;
        zB = (dPQK.z * 0.5 + pqk.z) / (dPQK.w * 0.5 + pqk.w);
        if (zB > zA)
            swap(zB, zA);

        hitPixel = permute ? pqk.yx : pqk.xy;
        hitPixel *= 1.0 / sizeTex;

        intersect = intersectDepth(hitPixel, zA, zB);
    }

    Q0.xy += dQ.xy * i;
    Q0.z = pqk.z;
    hitPoint = Q0 / pqk.w;
    iterationCount = i;

    return intersect;
}

float alpha(bool intersect,
            float iterationCount,
            float2 hitPixel,
            float3 hitPoint,
            float3 vsRayOrigin,
            float3 vsRayDirection,
            float3 normal)
{
    float alpha = 1.0;

    // Fade ray hits that approach the maximum iterations
    alpha *= 1.0 - (iterationCount / _Iterations);

    // Fade ray hits that approach the screen edge
    float screenFade = _ScreenEdgeFadeStart;
    float2 hitPixelNDC = (hitPixel * 2.0 - 1.0);
    float maxDimension = min( 1.0, max( abs( hitPixelNDC.x), abs( hitPixelNDC.y)));
    alpha *= 1.0 - (max( 0.0, maxDimension - screenFade) / (1.0 - screenFade));

    // Fade ray hits base on how much they face the camera
    float eyeFadeStart = _EyeFadeStart;
    float eyeFadeEnd = _EyeFadeEnd;
    if (eyeFadeStart > eyeFadeEnd)
        swap(eyeFadeStart, eyeFadeEnd);

    float eyeDirection = clamp(vsRayDirection.z, eyeFadeStart, eyeFadeEnd);
    alpha *= 1.0 - ((eyeDirection - eyeFadeStart) / (eyeFadeEnd - eyeFadeStart));

    // Fade ray hits based on distance from ray origin
    alpha *= 1.0 - clamp(distance(vsRayOrigin, hitPoint) / _MaxRayDistance,
                         0.0, 1.0);

    // Fade with intersection
    alpha *= intersect ? 1.0 : 0.0;

    return clamp(alpha, 0, 1);
}

vec4 ssr()
{
    float depth = texture(texDepth, ib.uv).r;
    vec3 origin = ib.viewRay * depth;
    vec3 normal = texture(texNormal, ib.uv).xyz;
    vec3 ray    = normalize(reflect(normalize(origin), normalize(normal)));

    float2 uv2   = ib.uv * sizeTex;
    float c      = (uv2.x + uv2.y) * 0.25;
    float jitter = mod(c, 1.0);

    vec2 hitPixel;
    vec3 hitPoint;
    float iters;

    bool hit = raytrace(origin, ray, jitter, hitPixel, hitPoint, iters);
    float a  = alpha(hit, iters, hitPixel, hitPoint, origin, ray, normal);

    return vec4(a > 0 ? texture(texColor, hitPixel).rgb :
                        texture(texColor, ib.uv).rgb, a);
}

void main(void)
{
    color = ssr();
}
