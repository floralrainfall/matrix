#version 330 core
out vec4 f_color;
  
in vec4 v_fcolor; // the input variable from the vertex shader (same name and same type)  
in vec3 v_fnormal;
in vec3 v_fmpos;
in vec2 v_fuv;

struct light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
};

#define MAX_LIGHTS 8
uniform light sun;
uniform light lights[MAX_LIGHTS];
uniform int lights_on = 0;
uniform vec3 view_position;

uniform sampler2D texture0;

vec3 calculateLight(light _light)
{
    vec3 norm = normalize(v_fnormal);
    vec3 light_dir = normalize(_light.position - v_fmpos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * sun.diffuse;

    vec3 view_dir = normalize(view_position - v_fmpos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = spec * sun.specular;

    vec3 result = _light.ambient + diffuse + specular;
    return result;
}

vec3 calculateSunLight(light _light)
{
    float sun_dist = 500.0;
    vec3 norm = normalize(v_fnormal);
    vec3 light_dir = normalize(_light.position * sun_dist - v_fmpos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * sun.diffuse;

    vec3 view_dir = normalize(view_position - v_fmpos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = spec * sun.specular;

    vec3 result = _light.ambient + diffuse + specular;
    return result;
}

void main()
{
    vec4 sample = texture2D(texture0, v_fuv);

    vec3 otherlights = vec3(0,0,0);

    for(int i = 0; i < lights_on; i++)
        otherlights += calculateLight(lights[i]);

    vec3 result = (calculateSunLight(sun) + otherlights) * sample.xyz;
    f_color = vec4(result, sample.a);
} 
