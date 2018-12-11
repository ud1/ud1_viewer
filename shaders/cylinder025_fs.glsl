#version 330

layout(location = 0) out vec4 outputColor;
in vec3 globalPos;
uniform mat4 VP;
uniform mat4 M;
uniform mat4 MINV;
uniform float radius;
uniform vec3 eyePosition;
uniform vec4 color;
uniform vec3 lightPos;

void main()
{
    float sphereRadius2 = 1;
    vec3 meyePosition = (MINV * vec4(eyePosition, 1)).xyz;
    vec3 rayDirection = globalPos - meyePosition;
    vec3 rayDirectionXY = vec3(rayDirection.xy, 0);
    float rayDirectionL = length(rayDirection);
    float rayDirectionXYL = length(rayDirectionXY);
    
    rayDirection /= rayDirectionL;
    rayDirectionXY /= rayDirectionXYL;
    
    vec3 relCenter = vec3(0.5, 0.5, 0) - meyePosition;
    float dt = dot(rayDirectionXY.xy, relCenter.xy);
    
    vec3 cr = cross(vec3(rayDirectionXY.xy, 0), vec3(relCenter.xy, 0));
	float d2 = dot(cr, cr);
	if (d2 > sphereRadius2)
        discard;
	
	float coef = rayDirectionL / rayDirectionXYL;
	float delta = sqrt(sphereRadius2 - d2);
	vec3 pointPos = meyePosition + ((dt + delta) * coef) * rayDirection;
	if (pointPos.x > 0.5 || pointPos.y > 0.5 || pointPos.z > 0.5 || pointPos.z < -0.5)
        discard;
    
    vec3 norm = -normalize(mat3(M) * vec3(pointPos.xy - vec2(0.5, 0.5), 0));
    pointPos = (M * vec4(pointPos, 1)).xyz;
    
    float wid = 0.03;
	vec3 pointPosMod = sign(mod(pointPos + vec3(wid, wid, wid) * 0.5, vec3(1, 1, 1)) - vec3(wid, wid, wid));
	float val = max(0.5, min(min(pointPosMod.x, pointPosMod.y), pointPosMod.z));
	
	float ang = dot(norm, normalize(lightPos - pointPos));
	outputColor = vec4((max(0, ang) + 0.3)*color.xyz * val, 1);
	
	vec4 realPos = VP * vec4(pointPos, 1);
	
	float ndcDepth = realPos.z / realPos.w;
    gl_FragDepth = ((gl_DepthRange.diff * ndcDepth) +
        gl_DepthRange.near + gl_DepthRange.far) / 2.0;

}
