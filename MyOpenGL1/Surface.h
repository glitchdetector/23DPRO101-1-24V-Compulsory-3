#pragma once
#include <cmath>
#include <vector>

#include "Types.h"
#include "Curve.h"
#include "Level.h"

class Surface
{
public:
	static void GenerateFromCurve(Curve* curve, Level* level, int subdivision, std::vector<Vertex>& vertices, std::vector<int>& indices)
	{
		float step_size = 1.0f / subdivision;
		for (float t = step_size; t < 1.0f; t += step_size)
		{
			glm::vec2 previousPoint = curve->getBezierPoint(t - step_size);
			glm::vec2 currentPoint = curve->getBezierPoint(t);
			float previousZ = level->GetGroundZAt2dCoord(previousPoint.x, previousPoint.y);
			float currentZ = level->GetGroundZAt2dCoord(currentPoint.x, currentPoint.y);
			{
				Vertex v;
				v.x = previousPoint.x;
				v.y = previousZ;
				v.z = previousPoint.y;
				v.u = 0.7f;
				v.v = 0.3f;
				v.r = 0.1f; v.g = 0.0f; v.b = 0.0f;
				vertices.push_back(v);
				indices.push_back(vertices.size() - 1);
			}
			{
				Vertex v;
				v.x = previousPoint.x;
				v.y = previousZ + 0.1f;
				v.z = previousPoint.y;
				v.u = 0.7f;
				v.v = 0.1f;
				v.r = 0.1f; v.g = 0.0f; v.b = 0.0f;
				vertices.push_back(v);
				indices.push_back(vertices.size() - 1);
			}
			{
				Vertex v;
				v.x = currentPoint.x;
				v.y = currentZ;
				v.z = currentPoint.y;
				v.u = 0.9f;
				v.v = 0.3f;
				v.r = 0.1f; v.g = 0.0f; v.b = 0.0f;
				vertices.push_back(v);
				indices.push_back(vertices.size() - 1);
			}
			{
				Vertex v;
				v.x = previousPoint.x;
				v.y = previousZ + 0.1f;
				v.z = previousPoint.y;
				v.u = 0.7f;
				v.v = 0.1f;
				v.r = 0.1f; v.g = 0.0f; v.b = 0.0f;
				vertices.push_back(v);
				indices.push_back(vertices.size() - 1);
			}
			{
				Vertex v;
				v.x = currentPoint.x;
				v.y = currentZ + 0.1f;
				v.z = currentPoint.y;
				v.u = 0.9f;
				v.v = 0.1f;
				v.r = 0.1f; v.g = 0.0f; v.b = 0.0f;
				vertices.push_back(v);
				indices.push_back(vertices.size() - 1);
			}
			{
				Vertex v;
				v.x = currentPoint.x;
				v.y = currentZ;
				v.z = currentPoint.y;
				v.u = 0.9f;
				v.v = 0.3f;
				v.r = 0.1f; v.g = 0.0f; v.b = 0.0f;
				vertices.push_back(v);
				indices.push_back(vertices.size() - 1);
			}
		}
	}

	static void GenerateSurface(Level* level, float min_x, float max_x, float min_y, float max_y, int subdivision, std::vector<Vertex>& vertices, std::vector<int>& indices)
	{
		float range_x = max_x - min_x;
		float range_y = max_y - min_y;
		float step_x = range_x / subdivision;
		float step_y = range_y / subdivision;

		for (float yy = min_y; yy < max_y; yy += step_y)
		{
			for (float xx = min_x; xx < max_x; xx += step_x)
			{
				{
					Vertex v;
					v.x = xx;
					v.y = level->GetGroundZAt2dCoord(xx, yy);
					v.z = yy;
					v.u = 0.7f;
					v.v = 0.9f;
					v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				}
				{
					Vertex v;
					v.x = xx;
					v.y = level->GetGroundZAt2dCoord(xx, yy + step_y);
					v.z = yy + step_y;
					v.u = 0.7f;
					v.v = 0.7f;
					v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				}
				{
					Vertex v;
					v.x = xx + step_x;
					v.y = level->GetGroundZAt2dCoord(xx + step_x, yy + step_y);
					v.z = yy + step_y;
					v.u = 0.9f;
					v.v = 0.9f;
					v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				}
				{
					Vertex v;
					v.x = xx;
					v.y = level->GetGroundZAt2dCoord(xx, yy);
					v.z = yy;
					v.u = 0.7f;
					v.v = 0.7f;
					v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				}
				{
					Vertex v;
					v.x = xx + step_x;
					v.y = level->GetGroundZAt2dCoord(xx + step_x, yy + step_y);
					v.z = yy + step_y;
					v.u = 0.9f;
					v.v = 0.9f;
					v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				}
				{
					Vertex v;
					v.x = xx + step_x;
					v.y = level->GetGroundZAt2dCoord(xx + step_x, yy);
					v.z = yy;
					v.u = 0.9f;
					v.v = 0.7f;
					v.r = 0.0f; v.g = 0.0f; v.b = 0.0f;
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				}
			}
		}
	}

};

