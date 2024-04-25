#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

uniform float timePassed;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 entityMatrix;

// Default Shader
void main()
{
	gl_Position = projection * view * entityMatrix * vec4(aPos, 1.0);
}