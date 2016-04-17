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

vec3 giUvw(vec3 worldPos)
{
    vec3 uvw = ((worldPos - boundsMin) / boundsSize).xzy;
    float zs = 1.0 / sizeTexGi.z;
    uvw.z    = 0.5 * zs + uvw.z * zs * (sizeTexGi.z - 1);
    return uvw;
}

void main()
{
    // Albedo and lightmap
    vec4 albedo   = texture(texAlbedo, ib.uv);
    vec3 light    = texture(texLight,  ib.uv).rgb;

    // GI
    vec3 uvwGi    = giUvw(ib.worldPos);
    vec3 gi       = texture(texGi, uvwGi).rgb;

    // View & light dir
    vec3 viewDir  = normalize(ib.worldPos - viewPos);
    vec3 incidVec = texture(texIncid, uvwGi).xzy;
    vec3 lightDir = normalize(incidVec);
    float incid   = smoothstep(0, 8, length(incidVec));

    // Ambient
    vec3 ambient  = 0.5f * albedo.rgb;

    // Diffuse
    vec3 diffuse  = 8.f * incid * albedo.rgb * max(dot(ib.normal, lightDir), 0.0);

    // Specular
    vec3 reflectDir = reflect(lightDir, ib.normal);
    float spec      = incid * pow(max(dot(viewDir, reflectDir), 0.0), 4.f);
    vec3 specular   = 16.f * albedo.rgb * light.r * spec;

    vec3 lighting = gi * ambient + gi * diffuse + gi * specular;
    vec4 color    = vec4(lighting, albedo.a);

    // OIT
    float weight = clamp(pow(min(1.0, color.a * 10.0) + 0.01, 3.0) *
                         1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    accum    = vec4(color.rgb * weight, color.a);
    reveal.r = color.a * weight;
}
