#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texLight;
uniform sampler2D texAo;
uniform sampler3D texGi;
uniform sampler3D texIncid;
uniform vec3      boundsMin;
uniform vec3      boundsSize;
uniform mat4      v;
uniform mat4      p;
uniform vec3      camPos;

// Const
vec3 sizeTexGi = textureSize(texGi, 0);

// Input
in Block
{
    vec2               uv;
    noperspective vec3 viewRay;
}
ib;

// Output
out vec4 color;

// Externals
float linearDepth(float depth, mat4 proj);
vec4 textureBicubic(sampler3D, vec3);

vec3 world(sampler2D depthSampler, vec2 uv, mat4 v, mat4 p)
{
  vec4 clip;
  clip.xy    = uv * 2.0 - 1.0;
  clip.z     = texture(depthSampler, uv).r * 2.0 - 1.0;
  clip.w     = 1.0;
  vec4 world = inverse(p * v) * clip;
  return world.xyz / world.w;
}

vec3 giUvw(vec3 worldPos)
{
    vec3 uvw = ((worldPos - boundsMin) / boundsSize).xzy;
    float zs = 1.0 / sizeTexGi.z;
    uvw.z    = 0.5 * zs + uvw.z * zs * (sizeTexGi.z - 1);
    return uvw;
}

float g1v(float dotNV, float k)
{
    return 1.0f / (dotNV * (1.0f - k) + k);
}

float ggx(vec3 N, vec3 V, vec3 L, float roughness, float F0)
{
    float alpha = roughness * roughness;
    vec3      H = normalize(V + L);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);

    // D
    float alphaSqr = alpha*alpha;
    float pi       = 3.14159f;
    float denom    = dotNH * dotNH * (alphaSqr - 1.0) + 1.0f;
    float D        = alphaSqr / (pi * denom * denom);

    // F
    float dotLH5   = pow(1.0 - dotLH, 5);
    float F        = F0 + (1.0 - F0) * (dotLH5);

    // V
    float k        = alpha / 2.0f;
    float vis      = g1v(dotNL, k) * g1v(dotNV, k);

    float specular = dotNL * D * F * vis;
    return specular;
}

void main(void)
{
    vec3 fragPos    = linearDepth(texture(texDepth, ib.uv).r, p) * ib.viewRay;
    vec3 worldPos   = world(texDepth, ib.uv, v, p);
    vec3 uvwGi      = giUvw(worldPos);
    vec3 uvwGiBic   = uvwGi - vec3(0.5 / sizeTexGi.xy, 0);
    vec3 gi         = textureBicubic(texGi, uvwGiBic).rgb;
    vec3 ao         = texture(texAo, ib.uv).r * gi.rgb;
    vec3 normal     = texture(texNormal, ib.uv).rgb;
    vec3 albedo     = texture(texColor,  ib.uv).rgb;
    vec3 light      = texture(texLight,  ib.uv).rgb;

    // View dir
    vec3 viewDir    = normalize(ib.viewRay);

    // Light dir & incident
    mat3 normalMat  = transpose(inverse(mat3(v)));
    vec3 incidVec   = texture(texIncid, uvwGi).xzy;
    vec3 incident   = normalize(incidVec);
    vec3 lightDir   = normalize(normalMat * incident);
    vec3 halfwayDir = normalize(lightDir + -viewDir);
    float incid     = smoothstep(0, 8, length(incidVec));

    // Ambient
    vec3 ambient    = 0.5f * albedo;

    // Diffuse
    vec3 diffuse    = 8.f * incid * albedo * max(dot(normal, lightDir), 0.0);

    // Specular
    float shininess = clamp(light.g, 0.0, 0.9);
    vec3 specular   = 16.f * incid * light.r * albedo *
                      ggx(normal, -viewDir, lightDir, 1.0 - shininess, 0.1);

    // Emissive
    vec3 emis       = 32.f * light.b * albedo;

    // Scattering
    vec3 scatter;
    const int steps   = 16;
    vec3 scatterRay   = normalize(camPos - worldPos);
    vec3 scatterStep  = scatterRay * 8.0;
    vec3 scatterPos   = worldPos;
    for (int i = 0; i < steps; ++i)
    {
        vec3 gi      = texture(texGi, giUvw(scatterPos)).rgb * 0.25;
        scatter     += gi / (steps + i * 32);
        scatterPos  += scatterStep;
    }

    vec3 lighting   = (ao * ambient   + ao   * diffuse) +
                      (ao * specular) + emis + scatter;
    color           = vec4(lighting, 1.0);
}
