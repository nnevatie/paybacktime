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
    float shininess = 4.0;
    // Phong
    vec3 reflectDir = reflect(lightDir, normal);
    float spec      = incid * pow(max(dot(viewDir, reflectDir), 0.0), 4.f);
    // Blinn-Phong
    //float spec      = incid * pow(max(dot(normal, halfwayDir), 0.0), shininess);
    float fresnel   = max(0, min(1, 0.f + 1.f *
                                (1.0 + dot(viewDir, normal)) * 1.f));

    vec3 specular   = 16.f * albedo * (light.r + fresnel) * spec;

    // Emissive
    vec3 emis       = 32.f * light.b * albedo;

    vec3 lighting   = (ao * ambient + ao * diffuse) + (ao * specular) + emis;
    color           = vec4(lighting, 1.0);
}
