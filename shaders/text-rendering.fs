#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;
uniform vec3 textColor;

void main()
{
	FragColor = vec4(textColor, texture(texture1, TexCoord).r);
}
