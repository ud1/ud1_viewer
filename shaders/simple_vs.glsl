#version 330 core

layout(location = 0) in vec3 fposition;

uniform mat4 MVP;
uniform vec3 ucolor;
out vec3 color;

void main()
{
    gl_Position = MVP * vec4(fposition, 1);
    color = ucolor;
}

