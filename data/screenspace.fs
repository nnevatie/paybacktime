#version 130

uniform vec4 in_color;

in  vec3 dist;
out vec4 out_color;

void main()
{
    vec3 n     = normalize(vec3(dFdx(gl_FragCoord.w), dFdy(gl_FragCoord.w), 1));
    vec3 l     = normalize(vec3(1, 10, 0));

    float d    = min(min(dist[0], dist[1]), dist[2]);
    float edge = exp2(-0.25 * d * d);

    vec4 color = vec4(in_color.xyz + 1000 * dot(n, l), 1);
    out_color  = mix(color, color + 0.5, edge);
}
