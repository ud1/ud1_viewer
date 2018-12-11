#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 MVP;
uniform float radius;
uniform vec3 normal;

out vec3 globalPos;

void main() {   
    vec3 p = gl_in[0].gl_Position.xyz;
    
    vec3 d1 = vec3(1, 0, 0);
    vec3 t1 = cross(normal, d1);
    if (dot(t1, t1) == 0)
        t1 = vec3(0, 1, 0);
    else
        t1 = normalize(t1);
        
    vec3 t2 = cross(normal, t1);
    
    t1 *= radius;
    t2 *= radius;
    
    vec3 fposition1 = p - t1 - t2;
    vec3 fposition2 = p + t1 - t2;
    vec3 fposition3 = p - t1 + t2;
    vec3 fposition4 = p + t1 + t2;    
    
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

