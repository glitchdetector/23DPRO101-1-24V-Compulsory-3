#pragma once

struct Vertex;

struct Entity
{
	glm::vec3 translation;
	glm::vec3 rotation;

	std::vector<Vertex> vertices;
	std::vector<int> indices;
};
