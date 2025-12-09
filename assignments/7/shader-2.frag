#version 330    

in vec3 ex_Color;
flat in int ex_InstanceId;

out vec4 out_Color;

uniform int colorCodeShader;

float rand11(float x) {
    return fract(sin(x) * 43758.5453);
}

void main(void) {
    switch(colorCodeShader) {
        case 0:
            out_Color = vec4(ex_Color, 1.0f);
            break;
        default:
            out_Color = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
