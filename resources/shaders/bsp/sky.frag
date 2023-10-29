#version 330 core
out vec4 f_color;
  
in vec4 v_fcolor; // the input variable from the vertex shader (same name and same type)  
in vec3 v_fnormal;
in vec3 v_fmpos;
in vec4 v_fpos;
in vec2 v_fuv;
in vec2 v_flm_uv;
in vec3 v_fvnorm;

struct light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
};

uniform sampler2D surface;
uniform sampler2D lightmap;
uniform samplerCube skybox;
uniform vec4 viewport;

void main()
{

} 