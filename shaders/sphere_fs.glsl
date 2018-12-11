#version 330

layout(location = 0) out vec4 outputColor;
in vec3 globalPos;
uniform mat4 MVP;
uniform float radius;
uniform vec3 eyePosition;
uniform vec3 position;
uniform vec3 lightPos;
uniform vec4 color;

void main()
{
	float sphereRadius2 = radius * radius;
	
	vec3 rayDirection = normalize(globalPos - eyePosition);
	
	vec3 relSphCenter = position - eyePosition;
	
	float dt = dot(rayDirection, relSphCenter);       
	
	vec3 cr = cross(rayDirection, relSphCenter);
	float d2 = dot(cr, cr);
	if (d2 > sphereRadius2)
        discard;
	
	float delta = sqrt(sphereRadius2 - d2);
	vec3 pointPos = eyePosition + ((dt - delta) > 0 ? (dt - delta) : (dt + delta)) * rayDirection;
	
	float wid = 0.03;
	vec3 pointPosMod = sign(mod(pointPos + vec3(wid, wid, wid) * 0.5, vec3(1, 1, 1)) - vec3(wid, wid, wid));
	float val = max(0.5, min(min(pointPosMod.x, pointPosMod.y), pointPosMod.z));
	
	float ang = dot(normalize(pointPos - position), normalize(lightPos - pointPos));
	outputColor = vec4((max(0, ang) + 0.3)*color.xyz*val, 1);
	
	vec4 realPos = MVP * vec4(pointPos, 1);
	
	float ndcDepth = realPos.z / realPos.w;
    gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) +
        gl_DepthRange.near + gl_DepthRange.far) / 2.0;

}
