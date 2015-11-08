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
    vec4 albedo     = texture(texColor, ib.uv);
    float ao        = texture(texAo, ib.uv).r * albedo.a;

    // Ambient
    vec3 ambient    = vec3(0.025);

    // Diffuse
    vec3 viewDir    = normalize(-fragPos);
    vec3 lightPos   = vec3(-10.0, 10.0, -100.0);
    vec3 lightDir   = normalize(lightPos - fragPos);
    vec3 diffuse    = max(albedo.rgb * ao,
                          albedo.rgb * max(dot(normal, lightDir), 0.0));

    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec      = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 specular   = 1.0 * vec3(spec);

    vec3 lighting   = ambient + ao * diffuse + ao * specular;
    color           = vec4(lighting, 1.0);
}
