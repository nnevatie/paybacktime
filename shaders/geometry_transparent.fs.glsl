#version 150

// Uniforms
uniform sampler2D texAlbedo;
uniform sampler2D texLight;
uniform sampler3D texGi;
uniform sampler3D texIncid;
uniform vec3      boundsMin;
uniform vec3      boundsSize;
uniform vec3      viewPos;

// Const
vec3 sizeTexGi = textureSize(texGi, 0);

// Input
in Block
{
    vec3 worldPos;
    vec3 normal;
    vec2 uv;
}
ib;

// Output
out vec4 accum;
out vec4 reveal;

// Externals
vec4 textureTricubic(sampler3D, vec3, vec3);
float ggx(vec3 N, vec3 V, vec3 L, float roughness, float F0);

vec3 giUvw(vec3 worldPos)
{
    return ((worldPos - boundsMin) / boundsSize).xzy;
}

void main()
{
    // Albedo and lightmap
    vec4 albedo   = texture(texAlbedo, ib.uv);
    vec3 light    = texture(texLight,  ib.uv).rgb;

    // GI
    vec3 uvwGi      = giUvw(ib.worldPos);
    vec3 gi         = textureTricubic(texGi, uvwGi, sizeTexGi).rgb;

    // View & light dir
    vec3 viewDir  = normalize(ib.worldPos - viewPos);
    vec3 incidVec = texture(texIncid, uvwGi).xzy;
    vec3 lightDir = normalize(incidVec);
    float incid     = smoothstep(0.0, 2.0, length(incidVec));

    // Ambient
    vec3 ambient  = 0.25f * albedo.rgb;

    // Diffuse
    vec3 diffuse  = 8.f * incid * albedo.rgb * max(dot(ib.normal, lightDir), 0.0);

    // Specular
    float shininess = clamp(light.g, 0.0, 0.9);
    vec3 specular   = 16.f * incid * light.r * albedo.rgb *
                      ggx(ib.normal, -viewDir, lightDir, 1.0 - shininess, 0.1);

    vec3 lighting = min(vec3(1.0), gi * ambient + gi * diffuse + gi * specular);
    vec4 color    = vec4(lighting, albedo.a);

    // OIT
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) *
                         1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    accum    = vec4(color.rgb * weight, color.a);
    reveal.r = color.a * weight;
}
