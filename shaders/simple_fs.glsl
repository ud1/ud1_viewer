#version 330 core

// Ouput data
layout(location = 0) out vec4 out_color;

in vec3 color;

void main()
{
    out_color = vec4(color, 1);
}
