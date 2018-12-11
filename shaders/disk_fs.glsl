#version 330

layout(location = 0) out vec4 outputColor;
in vec3 globalPos;
uniform float radius;
uniform vec3 position;
uniform vec4 color;

void main()
{
    outputColor = color;
	
	float rad = length(globalPos - position);
	if (rad > radius || rad < radius*0.8)
        discard;
}
