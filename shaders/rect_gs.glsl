#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 MVP;
uniform vec3 ht1;
uniform vec3 ht2;

out vec3 globalPos;
flat out vec3 norm;

void main() {   
    vec3 p = gl_in[0].gl_Position.xyz;
    
    vec3 fposition1 = p - ht1 - ht2;
    vec3 fposition2 = p + ht1 - ht2;
    vec3 fposition3 = p - ht1 + ht2;
    vec3 fposition4 = p + ht1 + ht2;
    norm = normalize(cross(ht1, ht2));
    
    gl_Position = MVP * vec4(fposition1, 1);
    globalPos = fposition1;
    EmitVertex();

    gl_Position = MVP * vec4(fposition2, 1);
    globalPos = fposition2;
    EmitVertex();
    
    gl_Position = MVP * vec4(fposition3, 1);
    globalPos = fposition3;
    EmitVertex();
    
    gl_Position = MVP * vec4(fposition4, 1);
    globalPos = fposition4;
    EmitVertex();

    EndPrimitive();
} 

