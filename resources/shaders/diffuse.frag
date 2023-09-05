#version 330 core
out vec4 f_color;
  
in vec4 v_fcolor; // the input variable from the vertex shader (same name and same type)  
in vec3 v_fnormal;
in vec3 v_fmpos;

struct sunlight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};

uniform sunlight sun;

void main()
{
    float sun_dist = 500.0;
    vec3 norm = normalize(v_fnormal);
    vec3 light_dir = normalize((sun_dist*vec3(-0.5,-0.5,0.5)) - v_fmpos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * sun.diffuse;
    
    vec3 result = (sun.ambient + diffuse) * vec3(v_fcolor);
    f_color = vec4(result, 1.0);
} 
