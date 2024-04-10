#version 330 core

in vec3 color;
in vec3 FragPos;
in vec2 texture_coord;
in vec3 Normal;

uniform bool bUseTexture;
uniform sampler2D texture_1;
uniform vec3 viewPos;
uniform vec3 playerPos;
uniform vec3 evilmanPos;

float getFogFactor(float d)
{
	const float FogMax = 40.0;
	const float FogMin = 10.0;

	if (d >= FogMax) return 1;
	if (d <= FogMin) return 0;

	return 1.0 - (FogMax - d) / (FogMax - FogMin);
}

void main()
{
	//vec3 viewDir = normalize(viewPos - FragPos);
	//gl_FragColor = vec4(color, 1.0);
	/*if (true) {
		gl_FragColor = vec4(viewPos, 1.0);
		return;
	}*/
	float viewDist = distance(viewPos, FragPos);

	float playerDist = distance(playerPos, FragPos) + 35.0;
	float evilmanDist = distance(evilmanPos, FragPos) + 35.0;
	float fogDist = min(evilmanDist, playerDist);

	vec4 emission = vec4(0.0, 0.0, 0.0, 0.0);
	if (bUseTexture) {
		gl_FragColor = texture(texture_1, texture_coord);
		emission = gl_FragColor * color.r;
	}
	else 
	{
		gl_FragColor = vec4(color, 1.0);
	}
	{
		float groundFog = min(0.0, FragPos.y);
		gl_FragColor -= vec4(groundFog, groundFog, groundFog, 0.0);
	}
	//float playerFog = min(0.0, playerPos.y);
	gl_FragColor *= 1.0 - getFogFactor(fogDist);
	gl_FragColor += emission / 2.0;
	//gl_FragColor = vec4(Normal, 0.0);
	//gl_FragColor *= vec4(viewDir, 1.0);
}