#version 330 core

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;

out vec4 ex_Color;

uniform int codColShader;

void main(void) {
  gl_Position = in_Position;
  if(codColShader == 1) {
    gl_Position = vec4(-in_Position.x, -in_Position.y, in_Position.y, in_Position.w);
  }
  ex_Color = in_Color;
}
