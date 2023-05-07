#version 330

out vec4 fragColor;

in vec2 TexCoords;

uniform sampler2D texture0;

void main()
{    
    fragColor = texture(texture0, TexCoords);
    // fragColor = vec4(0.5,1,1,1);
}