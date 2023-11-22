#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;

uniform mat4 model = mat4(1);
uniform mat4 view = mat4(1);
uniform mat4 projection = mat4(1);

out vec3 v_fpos;
flat out vec3 v_fcolor;

void main()
{
    v_fpos = v_position;
    v_fcolor = v_color;
    vec4 pos = projection * view * model * vec4(v_position, 1.0);
    gl_Position = pos;
}