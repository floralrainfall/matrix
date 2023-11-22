#version 330 core
out vec4 f_color;
  

in vec4 v_fcolor; // the input variable from the vertex shader (same name and same type)  
in vec3 v_fnormal;
in vec4 v_fvpos;
in vec3 v_fmpos;
in vec4 v_fpos;
in vec2 v_fuv;
in vec2 v_flm_uv;
in vec3 v_fvnorm;
in vec3 v_fraydir;

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
uniform vec3 camera_position;

uniform mat4 model = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 projection = mat4(1);
uniform mat4 view_inverse;

void main()
{
    vec3 i = normalize(v_fvpos.xyz);
    vec3 viewR = reflect(i, normalize(v_fnormal));
    vec3 worldR = mat3(view_inverse) * i;    
    f_color = texture(skybox, worldR);
} 