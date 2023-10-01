#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;
layout (location = 3) in vec2 v_lm_uv;

uniform mat4 model = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 projection = mat4(1);

out vec4 v_fcolor;
out vec3 v_fnormal;
out vec3 v_fmpos;
out vec3 v_fvnorm;
out vec4 v_fpos;
out vec2 v_fuv;
out vec2 v_flm_uv;

void main()
{
    v_fvnorm = mat3(view * model) * v_normal;
    v_fmpos = vec3(model * vec4(v_position, 1.0));
    mat4 pv = projection * view;
    vec4 pos = pv * model * vec4(v_position, 1.0);
    v_fpos = pos;
    gl_Position = pos;
    v_fcolor = vec4(0.5,0.5,0.5,1.0);
    v_fnormal = v_normal;
    v_fuv = v_uv;
    v_flm_uv = v_lm_uv;
}