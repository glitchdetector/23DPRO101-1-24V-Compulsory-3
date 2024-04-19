#version 330 core

in vec3 color;
in vec3 FragPos;
in vec2 texture_coord;
in vec3 Normal;

uniform float timePassed;
uniform sampler2D texture_0;
uniform sampler2D texture_1;

void main()
{
	float timeFactor = timePassed / 100.0;
	float heightFactor = (FragPos.y - 4.0) / 8.0;
	vec4 stars = texture(texture_0, texture_coord);
	vec4 noise = texture(texture_1, (texture_coord / 1) - vec2(timeFactor, sin(timeFactor)));
	//gl_FragColor = vec4(heightFactor, 0.0, 0.0, 1.0);
	//gl_FragColor = noise;
	gl_FragColor = stars * noise * heightFactor;
}