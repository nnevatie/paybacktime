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
uniform mat3      nm;
uniform mat4      wm;

// Const
vec3 sizeTexGi          = textureSize(texGi, 0);
const int SCATTER_STEPS = 20;

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
vec4 textureTricubic(sampler3D, vec3, vec3);
float ggx(vec3 N, vec3 V, vec3 L, float roughness, float F0);

vec3 world(sampler2D depth, vec2 uv, mat4 v, mat4 p)
{
  vec4 clip  = vec4(uv * 2.0 - 1.0, texture(depth, uv).r * 2.0 - 1.0, 1.0);
  vec4 world = wm * clip;
  return world.xyz / world.w;
}

vec3 giUvw(vec3 worldPos)
{
    return ((worldPos - boundsMin) / boundsSize).xzy;
}

vec3 scattering(vec3 start)
{
    vec3 ray  = normalize(camPos - start);
    vec3 step = 5.0 * ray;
    vec3 end  = start + SCATTER_STEPS * step;

    vec3 uvw0 = giUvw(start);
    vec3 uvw1 = giUvw(end);
    vec3 uvws = (uvw1 - uvw0) / SCATTER_STEPS;

    float weight = 0.01;
    vec3 scatter = vec3(0.0);
    for (int i = 0; i < SCATTER_STEPS && uvw0.z < 1.05; ++i)
    {
        vec3 gi  = texture(texGi, uvw0).rgb;
        scatter += weight * gi;
        uvw0    += uvws;
    }
    return scatter;
}

void main(void)
{
    vec3 fragPos    = linearDepth(texture(texDepth, ib.uv).r, p) * ib.viewRay;
    vec3 worldPos   = world(texDepth, ib.uv, v, p);
    vec3 uvwGi      = giUvw(worldPos);
    vec3 gi         = textureTricubic(texGi, uvwGi, sizeTexGi).rgb;

    vec3 ao         = texture(texAo, ib.uv).r * gi.rgb;
    vec3 normal     = texture(texNormal, ib.uv).rgb;
    vec3 albedo     = texture(texColor,  ib.uv).rgb;
    vec3 light      = texture(texLight,  ib.uv).rgb;

    // View dir
    vec3 viewDir    = normalize(ib.viewRay);

    // Light dir & incident
    vec3 incidVec   = texture(texIncid, uvwGi).xzy;
    vec3 incident   = normalize(incidVec);
    vec3 lightDir   = normalize(nm * incident);
    float incid     = smoothstep(0.0, 2.0, length(incidVec));

    // Ambient
    vec3 ambient    = 0.25 * albedo;

    // Diffuse
    vec3 diffuse    = 8.f * incid * albedo * max(dot(normal, lightDir), 0.0);

    // Specular
    float shininess = clamp(light.g, 0.0, 0.9);
    vec3 specular   = 16.f * incid * light.r * albedo *
                      ggx(normal, -viewDir, lightDir, 1.0 - shininess, 0.1);

    // Emissive
    vec3 emis       = 32.f * light.b * albedo;

    // Scattering
    vec3 scatter    = scattering(worldPos);

    // Final lighting
    vec3 lighting   = (ao * ambient   + ao   * diffuse) +
                      (ao * specular) + emis + scatter;

    color = vec4(lighting, 1.0);
}
