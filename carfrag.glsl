#version 330

in vec4 fragPos;
in vec4 N;
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D texture0;

vec3 kd = vec3(0.8, 0.8, 0.8);
vec3 ka = vec3(0.3, 0.3, 0.3);
vec3 ks = vec3(0.8, 0.8, 0.8);

vec3 I = vec3(2, 2, 2);

vec3 Iamb = vec3(0.8, 0.8, 0.8);

vec3 color = vec3(1,0.75,0.8);

uniform vec3 lightPosition[3];
uniform vec3 eyePos;


void main()
{    
    // fragColor = texture(texture0, TexCoords);
    vec3 normal = vec3(N);
    vec3 color = Iamb * ka * color;
    vec3 V = normalize(eyePos - vec3(fragPos));

	for(int i=0; i < 3; i++){

        vec3 L = normalize(lightPosition[i] - vec3(fragPos));
	    vec3 H = normalize(L + V);
        float NdotL = dot( normal, L);
	    float NdotH = dot( normal, H);

        float r2 = dot(lightPosition[i] - vec3(fragPos), lightPosition[i] - vec3(fragPos));

        vec3 diffuseColor = (1/r2) * I * max(0, NdotL) * color * kd;
		vec3 specularColor = (1/r2) * I * pow(max(0, NdotH), 400) * color * ks;

        color += diffuseColor + specularColor;
    }

    FragColor = vec4(color, 1);
}