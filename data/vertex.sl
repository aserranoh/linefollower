
uniform mat4 u_mvprojection;
uniform vec3 u_light0;
uniform vec3 u_light1;
uniform vec3 u_light2;
uniform vec3 u_light3;
uniform float u_attenuation;

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;
varying vec4 v_color;

const float c_zero = 0.0;
const float c_one = 1.0;

vec4
light_color(vec3 light)
{
    vec3 vl;
    float cos_phi;
    vec4 color = vec4(c_one, c_one, c_one, c_one);
    float attenuation = c_one;

    vl = light.xyz - a_position.xyz;
    if (u_attenuation != c_zero) {
        attenuation = c_one / (dot(vl, vl) * u_attenuation);
    }
    cos_phi = max(c_zero, dot(a_normal, normalize(vl)));
    color = color * cos_phi * attenuation;
    return color;
}

void
main()
{
    v_color = light_color(u_light0);
    v_color = v_color + light_color(u_light1);
    v_color = v_color + light_color(u_light2);
    v_color = v_color + light_color(u_light3);
    v_color.a = c_one;

    gl_Position = u_mvprojection * a_position;
    v_texcoord = a_texcoord;
}

