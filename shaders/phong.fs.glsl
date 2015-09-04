#version 150

struct LightProperties
{
    vec3 dir;
    vec4 ambColor;
    vec4 diffColor;
    vec4 specColor;
};

struct MaterialProperties
{
    vec4  ambColor;
    vec4  diffColor;
    vec4  specColor;
    float specExp;
};

// Uniforms
uniform	LightProperties    light;
uniform	MaterialProperties mat;

// Input
in Block
{
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
layout(location = 0) out vec4 frag;
layout(location = 1) out vec4 normal;

void main()
{
    vec4 c    = light.ambColor * mat.ambColor;
    vec3 n    = normalize(ib.normal);
    float ndl = max(dot(light.dir, n), 0.0);
    if (ndl > 0.0)
    {
        vec3 eye   = normalize(v_eye);
        vec3 refl  = reflect(-light.dir, n);
        float edr  = max(dot(eye, reflection), 0.0);
        float spec = edr > 0.0 ? pow(edr, mat.specExp) : 0.0;
        c         += light.diffColor * mat.diffColor * ndl +
                     light.specColor * mat.specColor * spec;
    }
    frag   = color;
    normal = vec4(n.x * 0.5 + 0.5, n.y * 0.5 + 0.5, n.z * 0.5 + 0.5, 1.0);
}
