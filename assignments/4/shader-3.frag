#version 330 core

in vec4 ex_Color;
in vec2 tex_Coord;
out vec4 out_Color;

uniform sampler2D myTexture;
uniform int colorCode;

void main(void) {
	if(colorCode == 1) {
		out_Color = ex_Color;
	} else {
		out_Color = mix(texture(myTexture, tex_Coord), ex_Color, 0.2);
	}
}
