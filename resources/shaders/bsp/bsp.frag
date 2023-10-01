#version 330 core
out vec4 f_color;
  
in vec4 v_fcolor; // the input variable from the vertex shader (same name and same type)  
in vec3 v_fnormal;
in vec3 v_fmpos;
in vec2 v_fuv;
in vec2 v_flm_uv;

struct light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
};

uniform sampler2D surface;
uniform sampler2D lightmap;

void main()
{
    vec4 samplet = texture2D(surface, vec2(v_fuv.x, -v_fuv.y));
    vec4 samplel = texture2D(lightmap, v_flm_uv);

    vec3 result = samplet.xyz * samplel.xyz;
    f_color = vec4(result, samplet.a);
} 