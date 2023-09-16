#version 330 core
out vec4 f_color;
  
in vec2 v_fuv;

uniform sampler2D texture;
uniform vec4 color;

void main()
{
    vec4 scolor = texture2D(texture, v_fuv);
    if(length(scolor.xyz) == 0)
        discard;
    f_color = scolor * color;
} 
