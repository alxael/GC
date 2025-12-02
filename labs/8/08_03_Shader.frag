//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul VIII - 08_03_Shader.frag|
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec3 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;

//  Variabile uniforme;
uniform int codCol;
uniform sampler2D myTexture;

void main(void)
{
vec4 ex_Color = vec4(ex_Color, 1.0);
    switch (codCol)
    {
        case 1: 
            out_Color=vec4(0.0, 0.0, 0.0, 1.0); 
            break;
        default:
            out_Color=ex_Color; 
            // out_Color = mix(texture(myTexture, tex_Coord), ex_Color, 0.1);
    }
}
 