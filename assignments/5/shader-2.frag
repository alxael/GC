#version 330	

in vec4 ex_Color;
out vec4 out_Color;

uniform int colorCodeShader;

void main(void) {
    switch(colorCodeShader) {
        case 1:
            out_Color = vec4(0.0, 0.0, 0.0, 0.0);
            break;
        default:
            out_Color = ex_Color;
    }
}