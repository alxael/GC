#version 330 core

in vec4 ex_Color;
out vec4 out_Color;

uniform int colorCode;

void main(void) {
	switch(colorCode) {
		case 0:
			out_Color = ex_Color;
			break;
		case 1:
			out_Color = vec4(0.0, 0.0, 0.0, 0.0);
			break;
		case 2:
			out_Color = vec4(1.0, 0.0, 1.0, 0.0);
			break;
		default:
			break;
	}
}
