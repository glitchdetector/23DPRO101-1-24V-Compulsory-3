#version 330 core

in vec3 color;
in vec3 FragPos;
in vec2 texture_coord;

uniform bool bUseTexture;
uniform sampler2D texture_1;
uniform vec3 viewPos;

void main()
{
	//vec3 viewDir = normalize(viewPos - FragPos);
	//gl_FragColor = vec4(color, 1.0);
	if (bUseTexture) {
		gl_FragColor = texture(texture_1, texture_coord);
	}
	else 
	{
		gl_FragColor = vec4(color, 1.0);
	}
}