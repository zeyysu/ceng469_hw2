#version 330

vec3 eyePos = vec3(0, 0, 2);
vec3 Iamb = vec3(0.8, 0.8, 0.8);

vec3 kd = vec3(0.8, 0.8, 0.8);
vec3 ka = vec3(0.3, 0.3, 0.3);
vec3 ks = vec3(0.8, 0.8, 0.8);

uniform vec3 lightPosition[5];
uniform vec3 color[5];
uniform int lightSize;

in vec4 fragPos;
in vec4 N;
out vec4 FragColor;

void main(void)
{

	vec3 normal = normalize(vec3(N));

	vec3 V = normalize(eyePos - vec3(fragPos));

	vec3 resColor =  Iamb * ka;

	for(int i=0; i < lightSize; i++){
		vec3 L = normalize(lightPosition[i] - vec3(fragPos));
		vec3 H = normalize(L + V);
		float NdotL = dot(normal, L);
		float NdotH = dot(normal, H);

		float r2 = dot(lightPosition[i] - vec3(fragPos), lightPosition[i] - vec3(fragPos));
		
		vec3 diffuseColor = (1/r2) * max(0, NdotL) * color[i] * kd;
		vec3 specularColor = (1/r2) * pow(max(0, NdotH), 400) * color[i] * ks;

		resColor += diffuseColor;
		resColor += specularColor;
	}


    FragColor = vec4(resColor, 1);
}
