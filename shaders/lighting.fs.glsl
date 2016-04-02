#version 150

// Uniforms
uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texLight;
uniform sampler2D texAo;
uniform sampler3D texGi;
uniform vec3      boundsMin;
uniform vec3      boundsSize;
uniform mat4      v;
uniform mat4      p;

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

void main(void)
{
    vec3 fragPos    = linearDepth(texture(texDepth, ib.uv).r, p) * ib.viewRay;

    vec3 worldPos   = world(texDepth, ib.uv, v, p);

    vec3 giUv       = ((worldPos - boundsMin) / boundsSize).xzy;
    giUv.xy        -= 0.5 / vec2(textureSize(texGi, 0).xy);

    vec3 gi         = 1.0 * pow(textureBicubic(texGi, giUv).rgb, vec3(1.0));

    vec3 ao         = texture(texAo, ib.uv).r * gi;
    vec3 normal     = texture(texNormal, ib.uv).rgb;
    vec3 albedo     = texture(texColor,  ib.uv).rgb;
    vec3 light      = texture(texLight,  ib.uv).rgb;

    // Ambient
    vec3 ambient    = vec3(0);

    // Light dir
    vec3 viewDir    = normalize(ib.viewRay);
    vec3 lightPos   = vec3(1.0, 1.0, 5.0);
    vec3 lightDir   = normalize(lightPos - viewDir);

    // Diffuse
    vec3 diffuse    = 1.0 * max(albedo * ao,
                                albedo * max(dot(normal, lightDir), 0.0));
    // Specular
    vec3 halfwayDir = normalize(lightDir + ib.viewRay);
    float spec      = 1.0 * pow(max(dot(normal, halfwayDir), 0.0), 2.0);
    vec3 specular   = light.r * vec3(spec);

    vec3 lighting   = ambient + ao * diffuse + ao * specular;
    color           = vec4(lighting, 1.0);
}
