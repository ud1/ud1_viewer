#version 330 core

// Ouput data
layout(location = 0) out vec4 out_color;

uniform vec4 color;

void main()
{
    out_color = color;
}
