#version 330 core
out vec4 f_color;
  
in vec2 v_fuv;

uniform sampler2D texture;
uniform vec4 color;

void main()
{
    vec4 samp = texture2D(texture, v_fuv);
    if(samp.a == 0)
        discard;
    f_color = samp * color;
} 
