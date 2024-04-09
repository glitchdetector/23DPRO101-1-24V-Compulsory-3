#pragma once
#include <cmath>
#include <vector>

#include "Level.h"
#include "Types.h"
#include "ObjectFileLoader.h"

inline float randomRange(float min, float max)
{
	float range = max - min;
	float modifier = RAND_MAX / range;
	float result = rand() / RAND_MAX;
	return min + result;
}

class Surface
{
public:
	// Function to calculate height on the surface
	static float GetGroundZAt2dCoord(float x, float y)
	{
		x /= 4;
		y /= 4;
		return sin(x) + (cos(y) / 2) + sin(y);
	}

	static void GenerateTrees(Level& level, float min_x, float max_x, float min_y, float max_y, int numTrees)
	{
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		ReadObjectFile("tree.obj", vertices, indices);
		for (int i = 0; i < numTrees; i++)
		{
			float tree_x = randomRange(min_x, max_x);
			float tree_y = randomRange(min_y, max_y);
			Entity* tree = new Entity();
			tree->vertices = vertices;
			tree->indices = indices;
			tree->transformation.x = tree_x;
			tree->transformation.z = tree_y;
			level.entities.push_back(tree);
		}
	}

	static void GenerateSurface(float min_x, float max_x, float min_y, float max_y, int subdivision, std::vector<Vertex>& vertices, std::vector<int>& indices)
	{
		float range_x = max_x - min_x;
		float range_y = max_y - min_y;
		float step_x = range_x / subdivision;
		float step_y = range_y / subdivision;

		for (float yy = min_y; yy < max_y; yy += step_y)
		{
			for (float xx = min_x; xx < max_x; xx += step_x)
			{
				Vertex v1;
				v1.x = xx;
				v1.y = GetGroundZAt2dCoord(xx, yy);
				v1.z = yy;
				v1.u = v1.x;
				v1.v = v1.z;
				Vertex v2;
				v2.x = xx;
				v2.y = GetGroundZAt2dCoord(xx, yy + step_y);
				v2.z = yy + step_y;
				v2.u = v2.x;
				v2.v = v2.z;
				Vertex v3;
				v3.x = xx + step_x;
				v3.y = GetGroundZAt2dCoord(xx + step_x, yy + step_y);
				v3.z = yy + step_y;
				v3.u = v3.x;
				v3.v = v3.z;

				Vertex v4;
				v4.x = xx;
				v4.y = GetGroundZAt2dCoord(xx, yy);
				v4.z = yy;
				v4.u = v4.x;
				v4.v = v4.z;
				Vertex v5;
				v5.x = xx + step_x;
				v5.y = GetGroundZAt2dCoord(xx + step_x, yy + step_y);
				v5.z = yy + step_y;
				v5.u = v5.x;
				v5.v = v5.z;
				Vertex v6;
				v6.x = xx + step_x;
				v6.y = GetGroundZAt2dCoord(xx + step_x, yy);
				v6.z = yy;
				v6.u = v6.x;
				v6.v = v6.z;

				vertices.push_back(v1);
				indices.push_back(vertices.size() - 1);
				vertices.push_back(v2);
				indices.push_back(vertices.size() - 1);
				vertices.push_back(v3);
				indices.push_back(vertices.size() - 1);
				vertices.push_back(v4);
				indices.push_back(vertices.size() - 1);
				vertices.push_back(v5);
				indices.push_back(vertices.size() - 1);
				vertices.push_back(v6);
				indices.push_back(vertices.size() - 1);
			}
		}
	}

};

