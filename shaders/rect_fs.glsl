#version 330

layout(location = 0) out vec4 outputColor;
in vec3 globalPos;
flat in vec3 norm;
uniform vec3 position;
uniform vec3 gridShift;
uniform vec3 lightPos;
uniform vec4 color;

void main()
{
   // outputColor = vec4(0.5, 0.5, 0.5, 1);
	
	float wid = 0.03;
	vec3 pointPosMod = sign(mod(globalPos + vec3(wid, wid, wid) * 0.5 + gridShift, vec3(1, 1, 1)) - vec3(wid, wid, wid));
	float val = max(0.5, min(min(pointPosMod.x, pointPosMod.y), pointPosMod.z));
	
	float ang = dot(norm, normalize(lightPos - globalPos));
	outputColor = vec4((max(0, ang) + 0.3)*color.xyz*val, 0.5);
}
