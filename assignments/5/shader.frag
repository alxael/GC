#version 330

in vec3 ex_Color; 

out vec3 out_Color;

uniform int colorCodeShader;

void main(void)
{
    switch (colorCodeShader)
    {
        case 1: 
            out_Color=vec3(0.0, 0.0, 0.0); 
            break;
        default: 
            out_Color=ex_Color;
    }
}