#version 150

// Const
const vec3 ldir         = vec3(0, -1, -1);
const vec3 diffuseColor = vec3(0.5, 0.5, 0.75);
const vec3 specColor    = vec3(1.0, 1.0, 1.0);

// Uniforms
uniform mat4 mv;

// Input
in Block
{
    vec3 eye;
    vec3 normal;
    vec2 uv;
    vec3 bc;
}
ib;

// Output
out vec4 color;
out vec4 normal;

float edge(vec3 bc)
{
    vec3 d  = fwidth(bc);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.5, bc);
    return min(min(a3.x, a3.y), a3.z);
}

void main()
{
    mat3 nm = transpose(inverse(mat3(mv)));
    vec3 n  = normalize(nm * ib.normal);

    float lambertian = 0.25 + max(dot(n, ldir), 0.0);
    float specular   = 0.0;

    if(lambertian > 0.0)
    {
        vec3 reflectDir = reflect(-ldir, n);
        vec3 viewDir    = normalize(-ib.eye);
        float specAngle = max(dot(reflectDir, viewDir), 0.0);
        specular        = pow(specAngle, 4.0);
    }

    //vec4 c = vec4(lambertian * diffuseColor + specular * specColor, 1.0);
    //color  = mix(c + 0.25, c, edge(ib.bc));

    color  = vec4(lambertian * diffuseColor + specular * specColor, 1.0);
    normal = vec4(n.x * 0.5 + 0.5, n.y * 0.5 + 0.5, n.z * 0.5 + 0.5, 1.0);
}
