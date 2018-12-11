#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

uniform mat4 MVP;
uniform float radius;

out vec3 globalPos;

const vec3 normals[6] = vec3[6](vec3(-1, 0, 0), vec3(1, 0, 0), vec3(0, -1, 0), vec3(0, 1, 0), vec3(0, 0, -1), vec3(0, 0, 1));
const vec3 t1s[6]     = vec3[6](vec3(0, -1, 0), vec3(0, 1, 0), vec3(1, 0, 0), vec3(-1, 0, 0), vec3(-1, 0, 0), vec3(1, 0, 0));
const vec3 t2s[6]     = vec3[6](vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 1, 0), vec3(0, 1, 0));

void main() {   
    for (int i = 0; i < 6; ++i)
    {
        vec3 norm = normals[i] * radius;
        vec3 t1 = t1s[i] * radius;
        vec3 t2 = t2s[i] * radius;
        vec3 p1 = gl_in[0].gl_Position.xyz + norm;
        vec3 fposition1 = p1 - t1 - t2;
        vec3 fposition2 = p1 + t1 - t2;
        vec3 fposition3 = p1 - t1 + t2;
        vec3 fposition4 = p1 + t1 + t2;    
        
        gl_Position = MVP * vec4(fposition1.xyz, 1);
        globalPos = fposition1;
        EmitVertex();

        gl_Position = MVP * vec4(fposition2, 1);
        globalPos = fposition2;
        EmitVertex();
        
        gl_Position = MVP * vec4(fposition3, 1);
        globalPos = fposition3;
        EmitVertex();
        
        gl_Position = MVP * vec4(fposition4.xyz, 1);
        globalPos = fposition4;
        EmitVertex();

        EndPrimitive();
	}
} 

