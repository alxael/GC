#version 330 core

in vec4 ex_Color;
out vec4 out_Color;

uniform int codColShader;

void main(void) {
	switch(codColShader) {
		case 0:
			out_Color = ex_Color;
			break;
		case 1:
			out_Color = vec4(1.0 - ex_Color.r, 1.0 - ex_Color.g, 1.0 - ex_Color.b, ex_Color.w);
			break;
		default:
			out_Color = vec4(1.0, 0.0, 1.0, 0.0);
	}
}
