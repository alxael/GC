#version 330

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec3 in_Color;

out vec3 ex_Color;

uniform mat4 view;
uniform mat4 projection;

void main(void) {
    gl_Position = projection * view * in_Position;
    ex_Color = in_Color;
}
