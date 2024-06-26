#version 150

// Uniforms
uniform sampler2D texAlbedo;

// Input
in Block
{
    vec3 normal;
    vec2 uv;
}
ib;

// Output
out vec4 color;

void main()
{
    vec3 albedo     = texture(texAlbedo, ib.uv).rgb;

    // Ambient
    vec3 ambient    = 0.25 * albedo;

    vec3 lightDir   = normalize(vec3(-0.5, 1.0, -0.25));
    vec3 diffuse    = 2.5 * albedo * max(dot(ib.normal, lightDir), 0.0);

    vec3 c          = pow(ambient + diffuse, vec3(1.0 / 2.2));
    color           = vec4(c, dot(c.rgb, vec3(0.299, 0.587, 0.114)));
}
