#version 130

uniform vec4 in_color;

out vec4 out_color;

void main()
{
    vec3 n    = normalize(vec3(dFdx(gl_FragCoord.w), dFdy(gl_FragCoord.w), 1));
    vec3 l    = normalize(vec3(1, 1, 0));
    float a   = 0.5;
    out_color = in_color + 50 * max(0.0, dot(n, l));
}
