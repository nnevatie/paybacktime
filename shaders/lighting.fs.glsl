#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texLight;
uniform sampler2D texAo;
uniform sampler2D texSc;
uniform sampler3D texGi;
uniform sampler3D texIncid;
uniform mat4      w;
uniform mat3      n;
uniform vec3      boundsMin;
uniform vec3      boundsSize;

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
float ggx(vec3 N, vec3 V, vec3 L, float roughness, float F0);
vec3 world(sampler2D depth, vec2 uv, mat4 w);
vec3 worldUvw(vec3 pos, vec3 boundsMin, vec3 boundsMax);

void main(void)
{
    vec3 worldPos   = world(texDepth, ib.uv, w);
    vec3 uvwGi      = worldUvw(worldPos, boundsMin, boundsSize);
    vec3 gi         = texture(texGi, uvwGi).rgb;

    vec3 ao         = texture(texAo, ib.uv).r * gi.rgb;
    vec3 normal     = texture(texNormal, ib.uv).rgb;
    vec3 albedo     = texture(texColor,  ib.uv).rgb;
    vec3 light      = texture(texLight,  ib.uv).rgb;

    // View dir
    vec3 viewDir    = normalize(ib.viewRay);

    // Light dir & incident
    vec3 incidVec   = texture(texIncid, uvwGi).xzy;
    vec3 incident   = normalize(incidVec);
    vec3 lightDir   = normalize(n * incident);
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
    vec3 scatter    = texture(texSc, ib.uv).rgb;

    // Final lighting
    vec3 lighting   = (ao * ambient   + ao   * diffuse) +
                      (ao * specular) + emis + scatter;

    color = vec4(lighting, 1.0);
}
