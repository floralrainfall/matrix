#version 330 core
out vec4 f_color;
  
in vec2 v_fuv;
in vec4 v_fcolor;

uniform sampler2D texture;
uniform vec4 color;
uniform vec4 background;
uniform vec4 outline;

void main()
{
    vec4 scolor = texture2D(texture, v_fuv) * color;
    float d = 0.001;
    vec4 s1 = texture2D(texture, v_fuv + vec2(d,0));
    vec4 s2 = texture2D(texture, v_fuv + vec2(-d,0));
    vec4 s3 = texture2D(texture, v_fuv + vec2(0,d));
    vec4 s4 = texture2D(texture, v_fuv + vec2(0,-d));
    vec4 ocolor = ((s1 + s2 + s3 + s4) / 4);
    if(length(ocolor.xyz) == 0)
        ocolor = vec4(0);
    else
	ocolor = outline;
    vec4 color = scolor;    
    if(length(color.xyz) == 0)
    	color = ocolor;
    if(length(color.xyz) == 0)
    	color = background;	
    f_color = color * v_fcolor;
} 
