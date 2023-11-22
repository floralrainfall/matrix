#version 330 core
layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_uv;
layout (location = 2) in vec3 v_color;

uniform mat4 uitransform = mat4(1);

uniform vec3 offset;
uniform vec2 scale;

out vec4 v_fcolor;
out vec2 v_fuv;

void main()
{
    vec3 upos = vec3(v_position * scale, 0) + offset;
    vec4 pos = uitransform * vec4(upos, 1);
    gl_Position = pos;
    v_fcolor = vec4(v_color, 1.0);
    v_fuv = v_uv;
}