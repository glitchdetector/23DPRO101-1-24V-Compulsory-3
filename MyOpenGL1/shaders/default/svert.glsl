#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

uniform float timePassed;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 entityMatrix;

out vec3 color;
out vec3 FragPos;
out vec2 texture_coord;
out vec3 Normal;

void main()
{
	gl_Position = projection * view * entityMatrix * vec4(aPos, 1.0);
	color = aCol;
	FragPos = vec3(entityMatrix * vec4(aPos, 1.0));
	texture_coord = vec2(aUV.x, aUV.y);
	Normal = vec3(transpose(inverse(entityMatrix))) * aNormal;
}