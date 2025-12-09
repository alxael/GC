#version 330 core

layout(location=0) in vec4 in_Position;
layout(location=1) in vec3 in_Color;
layout(location=2) in vec3 in_Normal;

uniform mat4 viewShader;
uniform mat4 projectionShader;
uniform vec3 obsShader;

out vec3 ex_Color;
out vec3 frag_Position;
out vec3 frag_Normal;

void main(void)
{
    gl_Position = projectionShader * viewShader * in_Position;

    frag_Position = vec3(in_Position);
    frag_Normal = normalize(in_Normal);

    ex_Color = in_Color;
}
