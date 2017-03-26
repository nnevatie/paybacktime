// Inspired and partially based on shader by Morgan McGuire and Michael Mara:
// http://casual-effects.blogspot.fi/2014/08/screen-space-ray-tracing.html
// The above is released as open source under the BSD 2-Clause License
// http://opensource.org/licenses/BSD-2-Clause
#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform mat4      v;
uniform mat4      p;
uniform mat4      pc;
uniform float     zNear;
uniform float     zFar;

// Const
vec2 sizeTex = textureSize(texDepth, 0);

int   ITER_MAX         = 20;
float RAY_DIST_MAX     = 50.0;
float PX_STRIDE        = 20.0;
float PX_STRIDE_CUTOFF = 500.0;
float PX_THICKNESS     = 5.0;
vec3  PROJ_PARAMS      = vec3(0, zNear, zFar);
float EDGE_FADE_START  = 0.75;
float EYE_FADE_START   = 0.0;
float EYE_FADE_END     = 1.0;

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Outputs
out vec3 uva;

void swap(inout float f0, inout float f1)
{
    float t = f0;
    f0 = f1;
    f1 = t;
}

float distSq(vec2 a, vec2 b)
{
    a -= b;
    return dot(a, a);
}

bool intersectDepth(vec2 uv, float zMin, float zMax)
{
    float cameraZ = texture(texDepth, uv).r;
    return -zMax >= cameraZ && -zMax <= cameraZ + PX_THICKNESS;
}

bool raytrace(vec3 rayOrigin,
              vec3 rayDir,
              float jitter,
              out vec2 hitUv,
              out vec3 hitPoint,
              out float iterCount)
{
    // Clip to the near plane
    float rayLen = (rayOrigin.z + rayDir.z * RAY_DIST_MAX) > -PROJ_PARAMS.y ?
                   (-PROJ_PARAMS.y - rayOrigin.z) / rayDir.z : RAY_DIST_MAX;
    vec3 rayEnd  = rayOrigin + rayDir * rayLen;

    // Homogeneous clip space
    vec4 h0 = pc * vec4(rayOrigin, 1.0);
    vec4 h1 = pc * vec4(rayEnd,    1.0);

    float k0 = 1.0 / h0.w,
          k1 = 1.0 / h1.w;

    // Interpolated homogeneous camera-space points
    vec3 q0 = rayOrigin * k0,
         q1 = rayEnd    * k1;

    // Screen-space endpoints
    vec2 p0 = h0.xy * k0,
         p1 = h1.xy * k1;

    // Degenerate line case
    p1 += distSq(p0, p1) < 0.0001 ? 0.01 : 0.0;
    vec2 delta = p1 - p0;

    // Permute
    bool permute = false;
    if (abs(delta.x) < abs(delta.y))
    {
        // Vertical
        permute = true; delta = delta.yx; p0 = p0.yx; p1 = p1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx   = stepDir / delta.x;

    // Derivatives of Q and k
    vec3  dq = (q1 - q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2  dp = vec2(stepDir, delta.y * invdx);

    // Pixel stride
    float strideScaler = 1.0 - min(1.0, -rayOrigin.z / PX_STRIDE_CUTOFF);
    float pixelStride  = 1.0 + strideScaler * PX_STRIDE;

    // Scale derivatives
    dp *= pixelStride; dq *= pixelStride; dk *= pixelStride;
    p0 += dp * jitter; q0 += dq * jitter; k0 += dk * jitter;

    float i, za = 0.0, zb = 0.0;

    // Track ray
    vec4 pqk  = vec4(p0, q0.z, k0);
    vec4 dpqk = vec4(dp, dq.z, dk);
    bool intersect = false;

    for(i = 0; i < ITER_MAX && !intersect; ++i)
    {
        pqk += dpqk;
        za   = zb;
        zb   = (dpqk.z * 0.5 + pqk.z) / (dpqk.w * 0.5 + pqk.w);
        if (zb > za) swap(zb, za);

        hitUv     = (permute ? pqk.yx : pqk.xy) * 1.0 / sizeTex;
        intersect = intersectDepth(hitUv, za, zb);
    }

    // Output results
    q0.xy    += dq.xy * i;
    q0.z      = pqk.z;
    hitPoint  = q0 / pqk.w;
    iterCount = i;
    return intersect;
}

float alpha(float iterCount,
            vec2 hitUv,
            vec3 hitPoint,
            vec3 vsRayOrigin,
            vec3 vsRayDir)
{
    // Iterations
    float attIter = 1.0 - (iterCount / ITER_MAX);

    // Screen edges
    vec2 hitNdc       = hitUv * 2.0 - 1.0;
    float dimMax      = min(1.0, max(abs(hitNdc.x), abs(hitNdc.y)));
    float attScrEdges = 1.0 - max(0.0, dimMax - EDGE_FADE_START) /
                                         (1.0 - EDGE_FADE_START);
    // Back faces
    vec3 normal       = normalize(texture(texNormal, hitUv).xyz);
    float attBackFace = smoothstep(-0.17, 0.0, dot(normal, -vsRayDir));

    // Camera facing
    float fadeStart = EYE_FADE_START;
    float fadeEnd   = EYE_FADE_END;
    if (fadeStart > fadeEnd) swap(fadeStart, fadeEnd);
    float eyeDir       = clamp(vsRayDir.z, fadeStart, fadeEnd);
    float attCamFacing = 1.0 - ((eyeDir - fadeStart) / (fadeEnd - fadeStart));

    // Origin-distance
    float attOriginDist = 1.0 - clamp(distance(vsRayOrigin, hitPoint) /
                                      RAY_DIST_MAX, 0.0, 1.0);

    return attIter * attBackFace * attScrEdges * attCamFacing * attOriginDist;
}

void main(void)
{
    vec3 origin  = ib.viewRay * texture(texDepth, ib.uv).r;
    vec3 normal  = texture(texNormal, ib.uv).xyz;
    vec3 ray     = normalize(reflect(normalize(origin), normalize(normal)));
    vec2 uv2     = ib.uv * sizeTex;
    float c      = (uv2.x + uv2.y) * 0.25;
    float jitter = mod(c, 1.0);

    vec2  hitUv;
    vec3  hitPoint;
    float iters;
    bool hit = raytrace(origin, ray, jitter, hitUv, hitPoint, iters);
    float a  = hit ? alpha(iters, hitUv, hitPoint, origin, ray) : 0.0;
    uva      = vec3(hitUv, a);
}
