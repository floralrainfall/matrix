#version 330 core
out vec4 f_color;
  
in vec2 v_fuv;

uniform sampler2D texture;

void main()
{
    f_color = texture2D(texture, v_fuv);
} 
