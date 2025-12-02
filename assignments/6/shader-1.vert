#version 330

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in mat4 modelMatrix;

out vec3 ex_Color;
out vec2 tex_Coord;
flat out int ex_InstanceId;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(void) {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * in_Position;
    ex_Color = in_Color;
    tex_Coord = vec2(1 - in_TexCoord.x, in_TexCoord.y);
    ex_InstanceId = gl_InstanceID;
}
