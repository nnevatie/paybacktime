#version 150

// Uniforms
uniform sampler2D texPosDepth;
uniform sampler2D texNormal;
uniform sampler2D texColor;
uniform sampler2D texAo;

// Input
in Block
{
    vec2 uv;
}
ib;

// Output
out vec4 color;

void main(void)
{
    vec3 fragPos    = texture(texPosDepth, ib.uv).rgb;
    vec3 normal     = texture(texNormal, ib.uv).rgb;
    vec3 diffuse    = texture(texColor, ib.uv).rgb;
    float ao        = texture(texAo, ib.uv).r;

    vec3 ambient    = vec3(0.1);
    vec3 viewDir    = normalize(-fragPos);

    // Diffuse
    vec3 lightPos   = vec3(1.0, 1.0, 1.0);
    vec3 lightDir   = normalize(lightPos - fragPos);
    diffuse         = max(dot(normal, lightDir), 0.0) * diffuse;

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec      = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 specular   = vec3(spec);

    vec3 lighting   = ambient + ao * diffuse + specular;
    color           = vec4(lighting, 1.0);
}
