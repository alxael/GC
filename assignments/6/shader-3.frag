#version 330    

flat in int ex_InstanceId;
out vec4 out_Color;

void main(void) {
    if (ex_InstanceId <= 2) {
        out_Color = vec4(0.87, 0.87, 0.87, 1.0);
    } else {
        out_Color = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
