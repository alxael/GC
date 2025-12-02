#version 330    

in vec3 ex_Color;
in vec2 tex_Coord;
flat in int ex_InstanceId;

out vec4 out_Color;

uniform int colorCodeShader;
uniform sampler2D myTexture;

float rand11(float x) {
    return fract(sin(x) * 43758.5453);
}

vec3 niceColor(vec2 uv, int id) {
    float h = rand11(float(id) * 17.0);

    h += (uv.x + uv.y) * 0.1;
    h = fract(h);

    float s = 0.6;
    float v = 0.9;

    float c = v * s;
    float x = c * (1.0 - abs(fract(h * 6.0) * 2.0 - 1.0));
    float m = v - c;

    vec3 rgb;
    if(h < 1.0 / 6.0)
        rgb = vec3(c, x, 0.0);
    else if(h < 2.0 / 6.0)
        rgb = vec3(x, c, 0.0);
    else if(h < 3.0 / 6.0)
        rgb = vec3(0.0, c, x);
    else if(h < 4.0 / 6.0)
        rgb = vec3(0.0, x, c);
    else if(h < 5.0 / 6.0)
        rgb = vec3(x, 0.0, c);
    else
        rgb = vec3(c, 0.0, x);

    return rgb + vec3(m);
}

void main(void) {
    switch(colorCodeShader) {
        case 0:
            if(ex_InstanceId == 0) {
                out_Color = texture(myTexture, tex_Coord);
            } else {
                out_Color = vec4(niceColor(tex_Coord, ex_InstanceId), 1.0);
            }
            break;
        default:
            out_Color = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
