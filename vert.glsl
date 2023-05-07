#version 330

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

uniform mat4 modelingMat;
uniform mat4 transMat;
uniform mat4 cps;

uniform int sampleSize;
uniform float coordMultiplier;
uniform int surfaceindex;
uniform int bezierX;
uniform int bezierY;


out vec4 fragPos;
out vec4 N;

void main(void)
{
	float xmin = (-0.5)* coordMultiplier;
	float ymin = (-0.5)* coordMultiplier;

	float xmax, ymax;

	if(bezierX >= bezierY){
		xmax =  (0.5)* coordMultiplier;
		ymax = ymin + (bezierY*(xmax-xmin))/bezierX;	
	} else{
		ymax =  (0.5)* coordMultiplier;
		xmax =  xmin + (bezierX*(ymax-ymin))/bezierY;	
	}

	float bIndy = floor(float(surfaceindex)/bezierX);
	float bIndx = surfaceindex - bezierX * bIndy;

	float xdist = xmax - xmin;
	float ydist = ymax - ymin;
	xmin = xmin + ((xdist)/(bezierX)) * bIndx;
	xmax = xmin + (xdist)/(bezierX);

	ymax = ymax - ((ydist)/bezierY) * bIndy;
	ymin = ymax - ((ydist)/bezierY);

	float sY = floor(float(gl_VertexID)/sampleSize);
	float sX = gl_VertexID - sampleSize * sY;

	float x = xmin + sX * ((xmax-xmin)/(sampleSize-1));
	float y = ymax - sY * ((ymax-ymin))/(sampleSize-1);

	mat4 MB = mat4(
		-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0
	);

	mat4 MBT = transpose(MB);

	float s = (x-xmin)/(xmax-xmin);
	float t = (ymax-y)/(ymax-ymin);

	vec4 T = vec4(t*t*t, t*t, t, 1);
	vec4 S = vec4(s*s*s, s*s, s, 1);

	vec4 dS = vec4(3*s*s, 2*s, 1, 0);
	vec4 dT = vec4(3*t*t, 2*t, 1, 0);

	float Zt = dot(S, MB * cps * MBT * dT);
	float Zs = dot( dS,  MB * cps * MBT * T);

	mat4 xcps = mat4(
		xmin, xmin+(xmax-xmin)*0.33,  xmin+(xmax-xmin)*0.66, xmax,
		xmin,  xmin+(xmax-xmin)*0.33,  xmin+(xmax-xmin)*0.66, xmax,
		xmin, xmin+(xmax-xmin)*0.33,  xmin+(xmax-xmin)*0.66, xmax,
		xmin,  xmin+(xmax-xmin)*0.33,  xmin+(xmax-xmin)*0.66, xmax
	 ); 

	 mat4 ycps = mat4(
		ymax,ymax,ymax,ymax,
		ymin + (ymax-ymin)* 0.66,ymin + (ymax-ymin)* 0.66,ymin + (ymax-ymin)* 0.66,ymin + (ymax-ymin)* 0.66,
		ymin + (ymax-ymin)*0.33, ymin + (ymax-ymin)*0.33,ymin + (ymax-ymin)*0.33,ymin + (ymax-ymin)*0.33,
		ymin, ymin, ymin, ymin
	 );

	x =  dot(S,MB * xcps * MBT * T);
	y = dot(S,MB * ycps * MBT * T);
	float z = dot(S * MB * cps * MBT , T);

	float xT = dot( S* MB * xcps * MBT , dT);
	float yT = dot(S * MB * ycps * MBT , dT);
	float xS = dot( dS *  MB * xcps * MBT , T);
	float yS =  dot( dS *  MB * ycps * MBT , T);

	vec3 n = cross(vec3(xT,yT,Zt),vec3(xS,yS,Zs));

	mat4 modelMatInvTr = transpose(inverse(modelingMat));

	N = modelMatInvTr * vec4(n,0);

	fragPos = modelingMat *  vec4(x,y,z,1.0);

	gl_Position = transMat * vec4(x,y,z,1.0);
}

