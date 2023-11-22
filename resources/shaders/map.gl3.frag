#version 330 core

in vec3 v_fpos;
flat in vec3 v_fcolor;

out vec4 f_color;

void main()
{
    f_color = vec4(v_fcolor, 1.0);
}