#version 330 core

uniform bool bUseTexture;
uniform sampler2D texture_0;
uniform vec3 viewPos;
uniform vec3 playerPos;
uniform vec3 evilmanPos;

// Wireframe shader
void main()
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}