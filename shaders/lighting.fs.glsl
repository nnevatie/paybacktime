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
uniform float     time;

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
    vec4 gi         = texture(texGi, uvwGi);

    vec3 ao         = texture(texAo, ib.uv).r * gi.rgb;
    vec3 normal     = texture(texNormal, ib.uv).rgb;
    vec3 albedo     = texture(texColor,  ib.uv).rgb;
    vec3 light      = texture(texLight,  ib.uv).rgb;

    // View dir
    vec3 viewDir    = normalize(ib.viewRay);

    // Light dir & incident
    vec4 incid      = texture(texIncid, uvwGi);
    vec3 incidVec   = incid.xzy;
    vec3 incident   = normalize(incidVec);
    vec3 lightDir   = normalize(n * incident);
    float incidStr  = smoothstep(0.0, 2.0, length(incidVec));

    // Ambient
    vec3 ambient    = 0.25 * albedo;

    // Diffuse
    vec3 diffuse    = 8.f * incidStr * albedo * max(dot(normal, lightDir), 0.0);

    // Specular
    float shininess = clamp(light.g, 0.0, 0.9);
    vec3 specular   = 16.f * incidStr * light.r * albedo *
                      ggx(normal, -viewDir, lightDir, 1.0 - shininess, 0.1);
    // Emissive
    vec3 emis       = 32.f * light.b * albedo;

    // Scattering
    vec3 scatter    = texture(texSc, ib.uv).rgb;

    // Pulse
    float frq       = incid.w;
    float pulse     = 1.f - gi.w * 0.5f * (1.f + sin(time * frq));

    // Final lighting
    vec3 lighting   = pulse * ((ao * ambient   + ao   * diffuse) +
                               (ao * specular) + scatter) + emis;

    color = vec4(lighting, 1.0);
}
