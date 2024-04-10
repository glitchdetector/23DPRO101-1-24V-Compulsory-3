#pragma once
#include <vector>

#include "glm/vec2.hpp"

class Curve
{
public:
    std::vector<glm::vec2> points;

    glm::vec2 getBezierPoint(float t) {
        std::vector<glm::vec2> points = this->points;
        auto const maxi = points.size() - 1;
        for (int i = 0; i != maxi; ++i)
        {
            auto const maxj = maxi - i;
            for (int j = 0; j < maxj; ++j)
                points[j] += t * (points[j + 1] - points[j]);
        }
        return points[0];
    }
};

